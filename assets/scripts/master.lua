--[[
    Master Script - Orchestrates all vehicle scripts

    This script is loaded ONCE by C++. It manages per-vehicle script instances,
    ensuring each vehicle has its own isolated script environment.

    Architecture:
    - C++ loads master.lua once
    - C++ calls master functions: attach_script(), update(), detach_script()
    - Master uses loadfile() to create isolated script instances per vehicle
    - Modules (abs, tcs) are singletons via require(), stateless
    - Vehicle scripts have their own local state, no collision between vehicles

    C++ API:
    - master.attach_script(vehicle_id, script_path, config) -> bool
    - master.update(vehicle_id, ctx)
    - master.detach_script(vehicle_id)
    - master.reload_all()
]]

local master = {}

-- Vehicle script instances: vehicle_id -> { env, update_fn, script_path, config }
local vehicles = {}

-- Per-vehicle persistent state (survives between frames, reset on attach)
local vehicle_states = {}

-- Create isolated environment for a script instance
-- Inherits globals but has its own local scope
local function create_script_env()
    local env = {}
    setmetatable(env, { __index = _G })
    return env
end

-- Attach a script to a vehicle (creates new isolated instance)
function master.attach_script(vehicle_id, script_path, config)
    -- Check if already attached
    if vehicles[vehicle_id] then
        print(string.format("[Master] Vehicle %d already has a script, detaching first", vehicle_id))
        master.detach_script(vehicle_id)
    end

    -- Load the script file as a chunk (does not execute yet)
    local chunk, err = loadfile(script_path)
    if not chunk then
        print(string.format("[Master] Failed to load '%s': %s", script_path, tostring(err)))
        return false
    end

    -- Create isolated environment for this vehicle's script
    local env = create_script_env()

    -- Inject config into the environment (script can access it as global 'config')
    env.config = config or {}

    -- Set the environment for the chunk (Lua 5.1 / LuaJIT)
    setfenv(chunk, env)

    -- Execute the chunk to define the script's functions in its environment
    local ok, err = pcall(chunk)
    if not ok then
        print(string.format("[Master] Error executing '%s': %s", script_path, tostring(err)))
        return false
    end

    -- Verify the script defined an update function
    if type(env.update) ~= "function" then
        print(string.format("[Master] Script '%s' missing update() function", script_path))
        return false
    end

    -- Store the script instance
    vehicles[vehicle_id] = {
        env = env,
        update_fn = env.update,
        init_fn = env.init,           -- Optional: called once after attach
        destroy_fn = env.destroy,     -- Optional: called before detach
        script_path = script_path,
        config = config or {},
    }

    -- Initialize per-vehicle persistent state
    vehicle_states[vehicle_id] = {}

    print(string.format("[Master] Attached script '%s' to vehicle %d", script_path, vehicle_id))

    -- Call init if defined
    if env.init then
        local init_ctx = {
            id = vehicle_id,
            config = config or {},
            state = vehicle_states[vehicle_id],
        }
        local ok, err = pcall(env.init, init_ctx)
        if not ok then
            print(string.format("[Master] init() error for vehicle %d: %s", vehicle_id, tostring(err)))
        end
    end

    return true
end

-- Update a vehicle's script
function master.update(vehicle_id, ctx)
    local v = vehicles[vehicle_id]
    if not v then
        -- No script attached to this vehicle, that's ok
        return
    end

    -- Inject per-vehicle state and config into context
    ctx.state = vehicle_states[vehicle_id]
    ctx.config = v.config

    -- Call the script's update function with context
    local ok, err = pcall(v.update_fn, ctx)
    if not ok then
        print(string.format("[Master] update() error for vehicle %d: %s", vehicle_id, tostring(err)))
    end
end

-- Detach script from a vehicle
function master.detach_script(vehicle_id)
    local v = vehicles[vehicle_id]
    if not v then
        return
    end

    -- Call destroy if defined
    if v.destroy_fn then
        local destroy_ctx = {
            id = vehicle_id,
        }
        local ok, err = pcall(v.destroy_fn, destroy_ctx)
        if not ok then
            print(string.format("[Master] destroy() error for vehicle %d: %s", vehicle_id, tostring(err)))
        end
    end

    vehicles[vehicle_id] = nil
    vehicle_states[vehicle_id] = nil  -- Clean up persistent state
    print(string.format("[Master] Detached script from vehicle %d", vehicle_id))
end

-- Reload all scripts (hot reload for development)
-- Preserves vehicle IDs but reloads script files
function master.reload_all()
    print("[Master] Reloading all scripts...")

    -- Collect current attachments
    local attachments = {}
    for vehicle_id, v in pairs(vehicles) do
        table.insert(attachments, {
            vehicle_id = vehicle_id,
            script_path = v.script_path,
            config = v.config,
        })
    end

    -- Clear require cache for modules (so they reload too)
    for name, _ in pairs(package.loaded) do
        if name:match("^modules/") then
            package.loaded[name] = nil
            print(string.format("[Master] Cleared module cache: %s", name))
        end
    end

    -- Detach all scripts
    for _, att in ipairs(attachments) do
        master.detach_script(att.vehicle_id)
    end

    -- Re-attach all scripts (loads fresh from disk)
    local success_count = 0
    for _, att in ipairs(attachments) do
        if master.attach_script(att.vehicle_id, att.script_path, att.config) then
            success_count = success_count + 1
        end
    end

    print(string.format("[Master] Reloaded %d/%d scripts", success_count, #attachments))
    return success_count
end

-- Get count of attached scripts
function master.get_vehicle_count()
    local count = 0
    for _ in pairs(vehicles) do
        count = count + 1
    end
    return count
end

print("[Master] Script orchestrator loaded")

return master