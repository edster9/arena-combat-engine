--[[
    ABS Module (Anti-lock Braking System)

    Reusable library for preventing wheel lockup during hard braking.
    Detects wheel slip via angular velocity and reduces brake pressure.

    Usage:
        local abs = require("modules/abs")
        abs.update(ctx)

    Context (ctx) structure:
        ctx.state       - Per-entity state (use ctx.state.abs for ABS data)
        ctx.telemetry   - Vehicle telemetry
        ctx.controls    - Controls to modify
        ctx.dt          - Delta time

    Per-entity config (set by parent script in ctx.state):
        ctx.state.abs_enabled    - Enable/disable ABS
        ctx.state.abs_min_speed  - Min speed for ABS activation
]]

local abs = {}

-- Default configuration (used if not set in ctx.state)
abs.default_slip_threshold = 0.15   -- Slip ratio to trigger ABS
abs.default_min_speed = 2.2         -- m/s (~5 mph)
abs.default_brake_reduction = 0.5   -- Max brake reduction

-- ============================================================================
-- SLIP CALCULATION
-- ============================================================================

-- Calculate slip ratio for a single wheel
-- Returns: slip_ratio (negative = wheel slower than ground = locking)
--          abs_slip (absolute value for threshold comparison)
local function calc_wheel_slip(wheel, vehicle_speed_ms)
    if not wheel then return 0, 0 end

    local angular_vel = wheel.angular_velocity or 0
    local radius = wheel.radius or 0.3  -- default 30cm radius

    -- Wheel linear speed (m/s)
    local wheel_speed = math.abs(angular_vel * radius)

    -- Avoid division by zero
    local max_speed = math.max(wheel_speed, vehicle_speed_ms, 0.1)

    -- Slip ratio: negative = wheel spinning slower than vehicle (lockup)
    local slip = (wheel_speed - vehicle_speed_ms) / max_speed

    return slip, math.abs(slip)
end

-- Get maximum negative slip across all wheels (worst lockup for ABS)
local function get_max_negative_slip(telemetry)
    if not telemetry.wheels then return 0, 0 end

    local min_slip = 0  -- Most negative slip
    local max_abs_slip = 0
    local vehicle_speed = telemetry.speed_ms or 0

    for i = 1, 4 do
        local wheel = telemetry.wheels[i]
        if wheel then
            local slip, abs_slip = calc_wheel_slip(wheel, vehicle_speed)
            -- Only consider negative slip (lockup)
            if slip < min_slip then
                min_slip = slip
                max_abs_slip = abs_slip
            end
        end
    end

    return min_slip, max_abs_slip
end

-- ============================================================================
-- ABS UPDATE
-- ============================================================================

-- Update ABS - call this every frame
-- Modifies ctx.controls.brake based on detected wheel lockup
function abs.update(ctx)
    -- Check if ABS is enabled for this entity
    local enabled = ctx.state.abs_enabled
    if enabled == nil then enabled = true end  -- Default enabled
    if not enabled then return end

    local controls = ctx.controls
    local telemetry = ctx.telemetry

    local brake_input = controls.brake or 0
    if brake_input <= 0.1 then
        return  -- No significant braking
    end

    local min_speed = ctx.state.abs_min_speed or abs.default_min_speed
    local speed = telemetry.speed_ms or 0
    if speed < min_speed then
        return  -- Allow lockup at low speed for full stop
    end

    -- Check for lockup (negative slip = wheel slower than ground)
    local slip, abs_slip = get_max_negative_slip(telemetry)

    local slip_threshold = abs.default_slip_threshold
    local brake_reduction = abs.default_brake_reduction

    if slip < -slip_threshold then
        -- Wheel is locking - reduce brake proportionally to slip severity
        local reduction = math.min(abs_slip * 2, brake_reduction)
        controls.brake = brake_input * (1 - reduction)
    end
end

-- Reset ABS state for an entity (call when vehicle spawns, etc.)
function abs.reset(ctx)
    if ctx.state then
        ctx.state.abs = nil
    end
end

return abs