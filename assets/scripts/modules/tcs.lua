--[[
    TCS Module (Traction Control System)

    Reusable library for preventing wheelspin during acceleration.
    Detects wheel slip via angular velocity and reduces throttle.

    Usage:
        local tcs = require("modules/tcs")
        tcs.update(ctx)

    Context (ctx) structure:
        ctx.state       - Per-entity state (use ctx.state.tcs for TCS data)
        ctx.telemetry   - Vehicle telemetry
        ctx.controls    - Controls to modify
        ctx.dt          - Delta time

    Per-entity config (set by parent script in ctx.state):
        ctx.state.tcs_enabled    - Enable/disable TCS
        ctx.state.tcs_min_speed  - Min speed for TCS activation
]]

local tcs = {}

-- Default configuration (used if not set in ctx.state)
tcs.default_slip_threshold = 0.10     -- Slip ratio to trigger TCS (10% slip)
tcs.default_min_speed = 2.0           -- m/s - allow more launch wheelspin
tcs.default_min_throttle = 0.15       -- Minimum throttle (never cut below 15%)
tcs.default_max_reduction = 0.85      -- Max throttle reduction (85% cut = 15% min)

-- Rate limiting for log output
tcs.last_log_time = {}                -- Per-entity last log time
tcs.log_interval = 0.5                -- Log at most every 0.5 seconds

-- ============================================================================
-- SLIP CALCULATION
-- ============================================================================

-- Calculate slip ratio for a single wheel
-- Returns: slip_ratio (positive = spinning faster than ground)
--          abs_slip (absolute value for threshold comparison)
local function calc_wheel_slip(wheel, vehicle_speed_ms)
    if not wheel then return 0, 0 end

    local angular_vel = wheel.angular_velocity or 0
    local radius = wheel.radius or 0.3  -- default 30cm radius

    -- Wheel linear speed (m/s)
    local wheel_speed = math.abs(angular_vel * radius)

    -- Avoid division by zero
    local max_speed = math.max(wheel_speed, vehicle_speed_ms, 0.1)

    -- Slip ratio: positive = wheel spinning faster than vehicle (wheelspin)
    local slip = (wheel_speed - vehicle_speed_ms) / max_speed

    return slip, math.abs(slip)
end

-- Get maximum slip across rear wheels only (front steerable wheels have bogus Jolt slip)
-- Uses Jolt's mLongitudinalSlip directly if available (wheel.slip)
local function get_max_positive_slip(telemetry, debug_entity)
    if not telemetry.wheels then return 0, 0 end

    local max_slip = 0
    local max_abs_slip = 0
    local vehicle_speed = telemetry.speed_ms or 0
    local using_jolt = false
    local slips = {}

    -- Only check rear wheels (3=RL, 4=RR) - front steerable wheels have incorrect slip
    for i = 3, 4 do
        local wheel = telemetry.wheels[i]
        if wheel then
            local slip, abs_slip
            -- Prefer Jolt's slip value (same as gear shift check uses)
            if wheel.slip ~= nil then
                slip = wheel.slip
                abs_slip = math.abs(slip)
                using_jolt = true
            else
                -- Fallback to calculated slip
                slip, abs_slip = calc_wheel_slip(wheel, vehicle_speed)
            end
            slips[i] = slip
            -- Only consider positive slip (wheelspin)
            if slip > max_slip then
                max_slip = slip
                max_abs_slip = abs_slip
            end
        end
    end

    -- Debug: show slip source on first call (still show all 4 for diagnostics)
    if debug_entity and not tcs.debug_printed then
        tcs.debug_printed = true
        for i = 1, 2 do
            local wheel = telemetry.wheels[i]
            if wheel and wheel.slip ~= nil then
                slips[i] = wheel.slip
            end
        end
        print(string.format("[TCS] Using rear wheels only: RL=%.2f RR=%.2f (front: FL=%.2f FR=%.2f)",
              slips[3] or 0, slips[4] or 0, slips[1] or 0, slips[2] or 0))
    end

    return max_slip, max_abs_slip
end

-- ============================================================================
-- TCS UPDATE
-- ============================================================================

-- Update TCS - call this every frame
-- Modifies ctx.controls.throttle based on detected wheelspin
function tcs.update(ctx)
    -- Check if TCS is enabled for this entity
    local enabled = ctx.state.tcs_enabled
    if enabled == nil then enabled = false end  -- Default disabled
    if not enabled then return end

    local controls = ctx.controls
    local telemetry = ctx.telemetry

    local throttle_input = controls.throttle or 0
    if throttle_input <= 0.1 then
        return  -- No significant throttle
    end

    local min_speed = ctx.state.tcs_min_speed or tcs.default_min_speed
    local speed = telemetry.speed_ms or 0

    -- Debug: show we're running with throttle
    -- print(string.format("[TCS] Entity %d: throttle=%.2f speed=%.2f", ctx.id, throttle_input, speed))

    if speed < min_speed then
        return  -- Allow wheelspin for launch
    end

    -- Check for wheelspin (positive slip = wheel faster than ground)
    local slip, abs_slip = get_max_positive_slip(telemetry, ctx.id == 0)

    local slip_threshold = tcs.default_slip_threshold
    local max_reduction = tcs.default_max_reduction
    local min_throttle = tcs.default_min_throttle

    if slip > slip_threshold then
        -- Gradual reduction: scale from threshold to 100% slip
        -- At threshold (0.15): no reduction
        -- At 1.0 (100% slip): max_reduction (0.6)
        local slip_excess = (slip - slip_threshold) / (1.0 - slip_threshold)
        slip_excess = math.min(slip_excess, 1.0)  -- Cap at 1.0
        local reduction = slip_excess * max_reduction

        local new_throttle = throttle_input * (1 - reduction)
        new_throttle = math.max(new_throttle, min_throttle)  -- Never below min

        -- Rate-limited logging
        local now = ctx.telemetry.time or 0
        local last_log = tcs.last_log_time[ctx.id] or 0
        if now - last_log >= tcs.log_interval then
            tcs.last_log_time[ctx.id] = now
            print(string.format("[TCS] Entity %d: thr %.0f%%->%.0f%% (slip=%.0f%%)",
                  ctx.id, throttle_input * 100, new_throttle * 100, slip * 100))
        end

        controls.throttle = new_throttle
    end
end

-- Reset TCS state for an entity (call when vehicle spawns, etc.)
function tcs.reset(ctx)
    if ctx.state then
        ctx.state.tcs = nil
    end
end

return tcs