# Arena Combat Engine - Development Roadmap

## Current Status (2025-12-31)

**Branch: `one-phase-physics-scripts`**

This branch represents a major architectural pivot from kinematic maneuver animation to physics-based execution with Lua scripting ("Reflex Scripts"). If successful, this will replace `main`.

### The Pivot: Why We're Changing

The previous approach used:
- **5-phase turns** - A tabletop board game convenience, not physics-realistic
- **Kinematic animation** - Predetermined arcs that ignore physics variance
- **Open-loop control** - Pre-recorded inputs that fail when physics diverges

The new approach:
- **Single-phase turns** - One maneuver per 1-second turn, physics-realistic
- **Real physics execution** - Actual drivetrain simulation (engine, gearbox, tires)
- **Closed-loop Reflex Scripts** - Lua scripts that adapt to physics in real-time

See [physics-based-turn-system.md](proposals/physics-based-turn-system.md) and [maneuver-scripting-system.md](proposals/maneuver-scripting-system.md) for full proposals.

---

## Phase 1: Cleanup - IN PROGRESS

### Tear Down Cinematic System
- [ ] Disable turn-based mode (flag off, keep code for reference)
- [ ] Remove 5-phase system completely
- [ ] Remove kinematic maneuver execution
- [ ] Clean up handling.cpp/h (archive for potential hybrid mode later)
- [ ] Remove ghost path preview (will rebuild with new system)

### Documentation
- [x] Create proposal documents
- [x] Update ROADMAP.md
- [ ] Update README.md

---

## Phase 2: Physics Restoration

### Transmission System
- [ ] Merge gearbox system from `engine-based-physics` branch
- [ ] Merge drivetrain (gear ratios, differential, shift logic)
- [ ] Remove `use_linear_accel` mode (no more fake constant force)
- [ ] Enable real engine torque curves

### Physics Tuning
- [ ] Get vehicle driving naturally in freestyle mode
- [ ] Tune tire friction for realistic handling
- [ ] Tune suspension for stability
- [ ] Validate acceleration matches target (10 mph/s class system)
- [ ] Test at various speeds (10, 30, 60 mph)

---

## Phase 3: Lua/Sol3 Integration

### Core Integration
- [ ] Add Sol3 dependency to CMake (header-only)
- [ ] Use LuaJIT backend for 2-10x performance over standard Lua
- [ ] Create `ReflexScriptEngine` class
- [ ] Implement sandbox environment (whitelist safe functions)
- [ ] Add execution time limits (kill runaway scripts)

**Why LuaJIT:**
- Drop-in replacement for Lua 5.1
- 2-10x faster execution (critical for 15 Hz script updates)
- Sol3 supports both standard Lua and LuaJIT
- Battle-tested in games (World of Warcraft, Garry's Mod, etc.)

### Telemetry API (Script Inputs)
```lua
state = {
    position = {x, y, z},
    heading = 0.0,           -- degrees
    speed = 0.0,             -- mph
    velocity = {x, y, z},
    yaw_rate = 0.0,          -- degrees/second
    slip_angle = {fl, fr, rl, rr},
    slip_ratio = {fl, fr, rl, rr},
    tire_load = {fl, fr, rl, rr},
    is_grounded = true,
    time_elapsed = 0.0,
    -- ... more
}
```

### Control API (Script Outputs)
```lua
controls = {
    steering = 0.0,      -- -1.0 to +1.0
    throttle = 0.0,      -- 0.0 to 1.0
    brake = 0.0,         -- 0.0 to 1.0
    e_brake = false,     -- true/false
}
```

### Helper Functions
- [ ] `clamp(value, min, max)`
- [ ] `lerp(a, b, t)`
- [ ] `normalize_angle(degrees)`
- [ ] `angle_diff(a, b)`
- [ ] `pid_controller(kp, ki, kd)` - PID helper object

---

## Phase 4: Script Lifecycle

### Always-On Execution
Scripts run continuously, not just during maneuvers. This enables:
- Traction control
- Stability control
- Autopilot / AI driving
- Automated testing

### Script Management
- [ ] Script loading from files
- [ ] Hot-reload support (edit and test without restart)
- [ ] Per-vehicle script contexts (isolated Lua states)
- [ ] Error handling and logging

### Built-in Scripts
- [ ] Traction Control System (TCS) - prevent wheel spin
- [ ] Stability Control (ESC) - prevent spins
- [ ] Simple Autopilot - drive straight, maintain speed

---

## Phase 5: Automated Testing Framework

Use the script system to write automated tests:
- [ ] Acceleration tests (0-60 time validation)
- [ ] Braking tests (60-0 distance)
- [ ] Handling tests (slalom, lane change)
- [ ] Stress tests (random inputs, find edge cases)

---

## Phase 6: Maneuver System

### Maneuver Definition
```lua
maneuver = {
    name = "Bootlegger Reverse",
    entry_speed = {min = 20, max = 35},  -- mph
    goal = {
        heading_change = 180,  -- degrees
        heading_tolerance = 15,
        max_final_speed = 5,   -- mph
    },
    script = "bootlegger.lua",
}
```

### Script Callbacks
- [ ] `init(goal)` - called once when maneuver starts
- [ ] `update(state, dt)` - called at 15 Hz, returns controls
- [ ] `is_complete(state)` - check success
- [ ] `is_failed(state)` - check failure

### Maneuver Templates
- [ ] STRAIGHT - no steering, maintain throttle
- [ ] DRIFT - slight constant steering angle
- [ ] BEND - steering proportional to desired angle
- [ ] BOOTLEGGER - full script with phases
- [ ] T_STOP - emergency brake with steering
- [ ] CONTROLLED_SKID - powerslide control

---

## Phase 7: UI and Editor

### Maneuver Workshop
- [ ] Timeline-based script visualizer
- [ ] Test run functionality
- [ ] Success rate tracking
- [ ] Save/load profiles

### Script Editor (Future)
- [ ] In-game code editor with syntax highlighting
- [ ] Console output for debugging
- [ ] Visual scripting option (node-based)

---

## Phase 8: Combat System

(Unchanged from previous roadmap)
- [ ] Weapon mounting and firing
- [ ] Damage resolution
- [ ] Armor and components

---

## Phase 9: Multiplayer

(Unchanged, but now easier with turn-based sync points)
- [ ] Network architecture
- [ ] Turn synchronization
- [ ] Script sharing (pre-match upload)

---

## Technical Architecture (New)

### Physics Layer
- **Jolt Physics** for rigid body simulation
- **Real drivetrain** - engine, gearbox, differential, wheels
- **Tire model** - slip angles, grip limits
- **No kinematic hacks** - physics determines outcomes

### Script Layer
- **Lua via Sol3** - fast, sandboxed scripting
- **15 Hz update rate** - scripts run every 66ms
- **Closed-loop control** - scripts read state, output corrections
- **Per-vehicle contexts** - isolated Lua states

### Game Logic
- **Single-phase turns** - one maneuver per 1-second turn
- **Simultaneous declaration** - all players declare, then execute
- **Physics-determined outcomes** - no dice rolls, physics decides
- **Optional Handling Status** - hybrid mode for tabletop feel

---

## References

- [Physics-Based Turn System Proposal](proposals/physics-based-turn-system.md)
- [Maneuver Scripting System Proposal](proposals/maneuver-scripting-system.md)
- Jolt Physics documentation
- Sol3 (Lua C++ bindings) documentation
- Classic tabletop vehicular combat rules