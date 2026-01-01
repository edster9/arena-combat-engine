# Arena Combat Engine

A 3D turn-based vehicular combat engine inspired by classic tabletop games.

## Project Status

**Branch: `one-phase-physics-scripts`** - Major architectural pivot in progress.

This branch is transitioning from kinematic maneuver animation to **physics-based execution with Lua scripting**. See [ROADMAP.md](docs/ROADMAP.md) for details.

**This is an educational/prototype project.** It is not affiliated with, endorsed by, or connected to any commercial game or trademark holder.

## What This Is

- A turn-based vehicular combat game with real physics execution
- Players plan maneuvers, then physics simulates the outcome
- **Reflex Scripts** (Lua) allow players to write adaptive control algorithms
- Built in C++ with SDL2, OpenGL, Jolt Physics, and Lua/Sol3

## What This Is NOT

- A real-time arcade game (not Twisted Metal)
- A racing simulator
- A commercial product

## The Vision: Reflex Scripts

Instead of predetermined animation arcs, maneuvers are executed by **Lua scripts** that run during physics simulation:

```lua
-- Example: Bootlegger Reverse script
function update(state, dt)
    local heading_error = angle_diff(state.heading, target_heading)

    if math.abs(heading_error) > 90 then
        -- Still rotating: full lock + e-brake
        return {
            steering = 1.0,
            throttle = 0.0,
            e_brake = true
        }
    else
        -- Stabilize: counter-steer
        return {
            steering = clamp(-state.yaw_rate * 0.02, -1, 1),
            throttle = 0.0,
            brake = 0.5
        }
    end
end
```

Scripts read vehicle telemetry (position, speed, slip angles) and output controls (steering, throttle, brake). This creates **closed-loop control** that adapts to physics variance - just like real vehicle stability systems.

## Current Features

### Physics
- Jolt Physics engine with wheeled vehicle simulation
- Real drivetrain: engine, gearbox, differential (in progress)
- Tire friction and slip modeling
- Collision detection with arena walls and obstacles

### Rendering
- SDL2 window with OpenGL 4.x context
- Chase camera with smooth follow and mouse orbit
- Arena with walls and obstacles
- Debug visualization for physics bodies

### Controls (Freestyle Mode)
- **W/S**: Throttle / Brake
- **A/D**: Steer left / right
- **Space**: E-brake
- **V**: Toggle cruise control
- **[/]**: Cruise speed down/up

### Camera
- **C**: Toggle chase camera
- **Right-click + drag**: Orbit camera
- **Scroll**: Zoom in / out

### General
- **P**: Toggle physics debug visualization
- **R**: Reload vehicle config
- **ESC**: Quit

## Building

### Ubuntu / Debian / WSL2

Install dependencies:

```bash
sudo apt update
sudo apt install build-essential cmake libsdl2-dev libglew-dev
```

Jolt Physics is fetched automatically by CMake.

Build and run:

```bash
cd client
./build.sh        # Build the client
./run.sh          # Run (auto-detects WSL2 GPU acceleration)
./run.sh --build  # Build and run in one step
```

## Project Structure

```
client/                 # Game client (C++/OpenGL)
  src/
    game/               # Game logic, config loaders
    physics/            # Jolt Physics integration
    render/             # Shaders, camera, mesh rendering
    ui/                 # UI panels and text
    platform/           # SDL2 window/input
    math/               # Vector and matrix math
    scripts/            # Lua script engine (planned)

assets/                 # Game assets
  data/
    vehicles/           # Vehicle JSON configs
    equipment/          # Chassis, power plants, tires, gearboxes
  config/scenes/        # Arena definitions
  shaders/              # GLSL shaders
  scripts/              # Lua reflex scripts (planned)

docs/                   # Design documents
  proposals/            # Architecture proposals
  ROADMAP.md            # Development status
  CONFIG_GUIDE.md       # Configuration reference

server/                 # Game server (Go) - planned
```

## Documentation

- [ROADMAP.md](docs/ROADMAP.md) - Development status and planned features
- [CONFIG_GUIDE.md](docs/CONFIG_GUIDE.md) - Vehicle and physics configuration
- [Physics-Based Turn System](docs/proposals/physics-based-turn-system.md) - Architecture proposal
- [Maneuver Scripting System](docs/proposals/maneuver-scripting-system.md) - Reflex Scripts proposal

## Development Phases

1. **Cleanup** - Remove kinematic system, disable turn mode (current)
2. **Physics Restoration** - Real drivetrain, remove linear force hack
3. **Lua Integration** - Sol3, telemetry API, control API
4. **Script Lifecycle** - Always-on scripts, hot reload
5. **Automated Testing** - Script-based physics tests
6. **Maneuver System** - Goal-based scripts with success/failure detection
7. **UI/Editor** - Maneuver workshop, script editor
8. **Combat** - Weapons, damage, armor
9. **Multiplayer** - Network sync, script sharing

## License

See [LICENSE](LICENSE) for details.

## Disclaimer

This is an independent project created for educational and personal use. It is inspired by classic tabletop vehicular combat games but is not affiliated with or endorsed by any game publisher.