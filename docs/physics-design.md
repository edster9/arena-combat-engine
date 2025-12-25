# Physics Design: Hybrid Car Wars + Real Physics

## Overview

This game blends **Car Wars tabletop rules** with **real-time physics simulation**.

- **Car Wars Rules** = The REFEREE (decides IF something bad happens)
- **Physics Engine** = REALITY (decides HOW it plays out)

---

## Core Philosophy

In traditional Car Wars, crash results are deterministic:
- Roll on Crash Table → "Major Skid" → Car rotates exactly 180°

In our hybrid system:
- Roll on Crash Table → "Major Skid" → Apply physics force → Let physics determine actual outcome
- Every crash is unique and visually interesting
- No two "major skids" look exactly the same

---

## Implementation Phases

### Phase 1: Teleport Execution (Current)
- Plan turn → Execute → Car teleports to end position
- No physics, just instant movement
- Good for testing UI and game flow

### Phase 2: Freestyle Physics Testbed
- Real-time driving mode (not turn-based)
- WASD controls for throttle/steering
- Hot keys to apply "invisible forces" simulating crash events
- Telemetry recording for debugging
- Goal: Fine-tune force magnitudes for each crash type

### Phase 3: Turn-Based Physics Integration
- Execute turn with full physics
- Car Wars rules trigger forces
- Physics simulates the result
- Camera follows action in real-time

---

## Crash Types and Physics Forces

| Crash Type | Car Wars Result | Physics Force to Apply |
|------------|-----------------|------------------------|
| Minor Skid | Slight slide | Small lateral impulse (500-1000 N) |
| Major Skid | 180° spin | Large lateral + angular torque |
| Severe Skid | Spin + roll chance | Above + potential upward tilt |
| Rollover | Car flips | Upward corner force + strong torque |
| Vault | Car airborne | Large upward impulse (launch) |

---

## Force Application Points

```
        FRONT
    ┌─────────────┐
    │  ●       ●  │  ← Front corners (for rollover)
    │             │
    │      ●      │  ← Center of mass (for lateral forces)
    │             │
    │  ●       ●  │  ← Rear corners
    └─────────────┘
        REAR

Minor Skid:  Force at center, perpendicular to heading
Major Skid:  Force at rear corner, creates spin
Rollover:    Upward force at one side
Vault:       Upward force at center
```

---

## Collision Handling

### Car Wars Rules (Damage Calculation)
- Ram damage = speed differential / 10 (dice)
- T-bone: Full damage to side, half to front
- Head-on: Both take full damage
- Sideswipe: Reduced damage, control roll

### Physics Response
- Let physics handle bounce/spin naturally
- Apply additional forces if control is lost
- Use Car Wars damage tables for HP loss

---

## Freestyle Testbed Design

### Controls
```
W/S     - Throttle/Brake
A/D     - Steering
SPACE   - Handbrake

1       - Apply Minor Skid force
2       - Apply Major Skid force
3       - Apply Rollover force
4       - Apply Vault force

R       - Reset car position
T       - Toggle telemetry display
P       - Pause physics
```

### Telemetry Recording
```c
typedef struct {
    double timestamp;
    Vec3 position;
    Vec3 velocity;
    Vec3 angular_velocity;
    float rotation_y;
    float speed_mph;
    Vec3 applied_force;
    Vec3 applied_torque;
} TelemetryFrame;
```

### Canned Simulations
1. Straight acceleration (0 to 60 mph)
2. High-speed straight + minor nudge
3. High-speed straight + major spin force
4. Ramp jump (vault simulation)
5. Wall collision at various angles

---

## Physics Engine Options

### Option A: Bullet Physics
- Industry standard, very robust
- C++ with C API available
- Handles complex collisions well
- Might be overkill for 2D-ish car game

### Option B: Custom Simple Physics
- Position, velocity, angular velocity
- Impulse-based collision response
- Friction model for tire grip
- Easier to tune for our specific needs

### Recommended: Start Custom, Upgrade if Needed
- Build simple rigid body physics first
- Add complexity as needed
- Can always swap in Bullet later

---

## Car Physics Model

```c
typedef struct {
    // Linear motion
    Vec3 position;
    Vec3 velocity;
    Vec3 acceleration;

    // Angular motion
    float rotation_y;        // Yaw (main rotation)
    float angular_velocity;  // Spin rate
    float angular_accel;

    // Properties
    float mass;              // kg
    float inertia;           // Rotational inertia
    float friction;          // Tire grip coefficient
    float drag;              // Air resistance

    // State
    bool on_ground;
    bool is_rolling;
    float roll_angle;        // For rollover
} CarPhysics;
```

---

## Integration with Car Wars Rules

```
TURN EXECUTION FLOW:

1. Planning Phase (current implementation)
   - Player selects speed change
   - Player selects maneuvers per phase
   - Preview ghost path

2. Execution Phase (to implement)
   For each phase:
   a. Calculate HC cost of maneuver
   b. If HC < 0: Roll control
   c. If control failed: Roll crash table
   d. Apply appropriate physics force
   e. Step physics simulation (~1 second)
   f. Check for collisions
   g. Apply collision damage (Car Wars rules)

3. Resolution Phase
   - Update car final position from physics
   - Update speed based on physics velocity
   - Apply damage to cars
   - Regain handling (end of turn)
```

---

## Future Game Mode Ideas

### Freestyle Test Drive Mode
Beyond physics tuning, freestyle driving could become an actual game mode:

1. **Car Builder Test Track**
   - After building a custom car, test it in real-time
   - WASD controls, fire weapons, make jumps
   - Solo mode to learn your car's handling before arena combat

2. **Quick Play Mode**
   - Faster turn-based variant (shorter planning time)
   - Or hybrid: plan maneuver, watch physics execute in real-time
   - Good for solo practice or quick matches

3. **Stunt Mode**
   - Score points for jumps, spins, near-misses
   - Test track with ramps and obstacles
   - Leaderboards for best stunts

---

## Next Steps

1. [x] Document physics design (this file)
2. [x] Implement teleport execution (simple)
3. [x] Add "Execute Turn" button to UI
4. [ ] Build freestyle physics testbed
5. [ ] Implement basic car physics (velocity, friction)
6. [ ] Add force application system
7. [ ] Create telemetry recording/playback
8. [ ] Run canned simulations, tune forces
9. [ ] Integrate physics into turn-based execution
