# BeamNG.drive Mod System Analysis

**Date:** 2025-12-30
**Purpose:** Evaluate BeamNG.drive as a potential platform for tabletop-style turn-based gameplay

## BeamNG.drive Overview

BeamNG.drive is a **soft-body physics simulator** with vehicles modeled as node-beam networks (700+ nodes, 4000+ beams per vehicle). The physics runs at **2000 Hz** for real-time deformation and damage simulation.

### Technical Architecture

- **Physics Core**: Closed-source C++ for performance
- **Vehicle Simulation**: Open-source Lua (modifiable)
- **Engine Base**: Modified Torque 3D
- **Node-Beam Format**: JBeam (JSON-like text files)

### Modding Capabilities

- Lua scripting for vehicle logic and gameplay
- HTML/JS for UI overlays
- Extension system with event hooks
- Scenario scripting for custom game modes
- Active modding community with weapons mods (Universal Weapons)

### Time Control Features

- `J` key: Pause physics for all vehicles
- `Alt + Arrow Keys`: Slow motion (up to 100x slowdown)
- Lua scripts can trigger conditional slowdown

## The Turn-Based Problem

**BeamNG cannot support true turn-based gameplay.** The fundamental architecture is incompatible.

### Comparison Table

| Aspect | BeamNG.drive | tabletop Arena |
|--------|--------------|----------------|
| Physics | Continuous soft-body at 2000 Hz | Kinematic animation + Jolt for collision |
| Movement | Real-time input (throttle/steering) | Discrete maneuvers (D0-D7) selected beforehand |
| Planning | None - drive in real-time | Simultaneous planning, phased execution |
| Damage | Emergent from deformation | Tabletop tables (armor, components) |
| Predictability | Chaotic (physics-driven) | Deterministic (maneuver arcs) |

### Key Limitations

1. **No physics stepping** - Can pause globally or slow time, but cannot pause individual vehicles or step physics frame-by-frame
2. **No kinematic override** - Vehicles always move via forces, not pre-computed paths
3. **Real-time only** - Engine architecture assumes continuous simulation
4. **Soft-body coupling** - Deformation is continuous; cannot "plan" damage

### Modding Limitations

Even with extensive Lua access:
- Cannot override the core physics loop
- Cannot implement discrete turn phases
- Cannot do simultaneous-reveal planning
- Cannot apply tabletop handling rolls or difficulty values

The Universal Weapons mod demonstrates combat is possible, but it's **real-time combat** - AI chase mode, aimable turrets, continuous fire. No mechanism exists for turn-based tactical allocation.

## Verdict: Not a Good Fit

BeamNG's strength (emergent soft-body physics) is orthogonal to tabletop' design (deterministic tabletop mechanics).

### What Would Be Lost

- Tabletop "feel" of planned maneuvers
- Handling Status and control rolls
- Difficulty values (D0-D7)
- Phased simultaneous execution
- Tactical weapon allocation
- Predictable movement arcs

### What Would Be Gained

- Pretty soft-body crashes
- Real-time driving feel
- Large existing vehicle library

The tradeoff is not favorable for a faithful tabletop implementation.

## Alternative Approach (Limited Use Case)

BeamNG could theoretically be used for **crash cinematics only**:

1. Execute turns in the tabletop engine
2. Export vehicle state at crash moment
3. Replay collision in BeamNG for soft-body destruction visualization
4. Record result as a visual effect

This is technically possible but complex to implement and provides only visual polish, not gameplay.

## Conclusion

**Build the custom engine.** The current Arena Combat Engine architecture with:
- Jolt Physics for collision detection
- Kinematic animation for maneuver execution
- Multi-phase turn system
- Ghost path preview

...is the correct approach for tabletop. BeamNG solves a fundamentally different problem (real-time driving simulation) and cannot be adapted to turn-based tactical gameplay without losing what makes both products valuable.

## References

- [BeamNG Physics Architecture](https://www.beamng.com/game/about/physics/)
- [BeamNG Programming Documentation](https://documentation.beamng.com/modding/programming/)
- [BeamNG Extensions System](https://documentation.beamng.com/modding/programming/extensions/)
- [BeamNG Modding Guidelines](https://beamng.com/game/support/policies/modding-guidelines/)
- [Universal Weapons Mod (GitHub)](https://github.com/stefan750/BeamNG-UniversalWeapons)
- [BeamNG Forum: Pause Physics Discussion](https://www.beamng.com/threads/pause-physics-for-selected-car.12672/)
- Classic tabletop vehicular combat rules (reference material)
