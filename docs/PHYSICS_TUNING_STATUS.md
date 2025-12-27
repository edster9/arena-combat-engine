# Physics Tuning Status

## Current State (2025-12-27)

We migrated from ODE to Jolt Physics and are now tuning vehicle acceleration to match Car Wars rules.

## Test Vehicle Configuration
- **Chassis**: mid_sized (4.6m x 1.8m x 1.4m)
- **Power Plant**: electric_large (power_factors = 2000)
- **Total Weight**: 2620 lbs (1188 kg)
- **Tires**: standard + radial modifier (mu = 1.6)
- **Drivetrain**: RWD

## Car Wars Expected Behavior
Based on power_factors / weight ratio:
- **Ratio**: 2000 / 2620 = 0.76
- **Acceleration Class**: 10 mph/turn (since 0.5 <= ratio < 1.0)
- **Expected Acceleration**: 10 mph/s = **4.47 m/s²**
- **Top Speed**: 360 * 2000 / (2000 + 2620) = **156 mph**
- **0-60 mph time**: ~6.0 seconds

## Current Physics Results (with power_factors = 2000)
- **Measured Acceleration**: ~3.2 m/s²
- **0-60 mph time**: ~8.4 seconds
- **Reaching**: ~72% of expected acceleration

## Key Files Modified
- `client/src/physics/jolt_physics.cpp` - Vehicle physics, acceleration test
- `client/src/physics/jolt_physics.h` - Added accel test state to PhysicsVehicle
- `client/src/game/config_loader.cpp` - Weight calculation (chassis + tires + power plant)
- `client/src/game/equipment_loader.cpp` - Tire physics (mu values)
- `assets/data/equipment/power_plants.json` - Power plant definitions
- `assets/data/equipment/tires.json` - Tire friction values

## Physics Parameters Currently Set
In `jolt_physics.cpp`:
- **Clutch strength**: `(maxTorque / engineInertia) * 2.0f`
- **Engine inertia**: `0.5f + (maxTorque / 1000.0f)`
- **Tire friction curves**: Peak at 6% slip (mu * 1.2), sliding at 20% (mu * 1.0)
- **Gear ratios**: 2.66, 1.78, 1.3, 1.0, 0.74
- **Differential ratio**: 3.42

## Acceleration Test Feature
Built-in test that triggers when you floor throttle from stopped:
- Prints speed, distance, avg acceleration every 0.5s
- Ends on collision, throttle release, or stop
- Shows 0-60 mph estimate

## Next Steps
1. **Tune physics to match Car Wars 10 mph/s acceleration**
   - Currently at 3.2 m/s², need 4.47 m/s² (increase by ~40%)
   - Options: increase torque multiplier, adjust clutch, modify friction curves

2. **Validate underpowered behavior**
   - Cars with ratio < 0.33 should not move
   - Currently they still accelerate (power=25 test moved at 1.7 m/s²)

3. **Test other acceleration classes**
   - 5 mph/s (ratio 0.33-0.5)
   - 15 mph/s (ratio >= 1.0)

## Quick Test Commands
```bash
cd client && ./build.sh && ./run.sh
```
- Up arrow = throttle
- R = respawn (resets acceleration test)
- Hot reload configs without restart (check key binding)
