# Architecture: Arena Duel Engine

## Overview

This document describes the technical architecture for the Car Wars Arena Duel demo. The goal is to build a minimal but extensible engine that can grow with the project.

---

## Technology Stack

| Layer | Choice | Rationale |
|-------|--------|-----------|
| **Language** | C (C99/C11) | Performance, control, learning value |
| **Windowing/Input** | SDL3 | Cross-platform, modern, GPU abstraction |
| **Rendering** | OpenGL 3.3+ | WSL2 compatible, well-documented |
| **Math** | Custom or cglm | Keep dependencies minimal |
| **Build** | CMake or Make | Standard, cross-platform |
| **Audio** | SDL3 Audio (later) | Integrated with SDL |
| **Physics** | Custom (simple) | Turn-based doesn't need Bullet/Box2D |

---

## Project Structure

```
car-wars-arena/
│
├── CMakeLists.txt              # Build configuration
├── README.md                   # Project overview
│
├── docs/                       # Documentation (this folder)
│   ├── vision.md
│   ├── game-design.md
│   ├── architecture.md
│   ├── milestones.md
│   └── assets.md
│
├── src/                        # Source code
│   ├── main.c                  # Entry point
│   │
│   ├── core/                   # Engine core
│   │   ├── engine.h/c          # Main engine lifecycle
│   │   ├── input.h/c           # Input handling
│   │   ├── time.h/c            # Timing, delta time
│   │   └── log.h/c             # Logging utilities
│   │
│   ├── math/                   # Math library
│   │   ├── vec2.h/c            # 2D vectors
│   │   ├── vec3.h/c            # 3D vectors
│   │   ├── mat4.h/c            # 4x4 matrices
│   │   └── transform.h/c       # Position/rotation/scale
│   │
│   ├── render/                 # Rendering system
│   │   ├── renderer.h/c        # Main renderer
│   │   ├── shader.h/c          # Shader loading/management
│   │   ├── mesh.h/c            # Mesh loading/rendering
│   │   ├── texture.h/c         # Texture loading
│   │   ├── camera.h/c          # Camera (orbit, etc.)
│   │   └── debug_draw.h/c      # Debug lines, shapes
│   │
│   ├── game/                   # Game-specific logic
│   │   ├── game.h/c            # Game state machine
│   │   ├── arena.h/c           # Arena definition, grid
│   │   ├── vehicle.h/c         # Vehicle data, state
│   │   ├── weapon.h/c          # Weapon definitions
│   │   ├── turn.h/c            # Turn structure, phases
│   │   ├── combat.h/c          # Combat resolution
│   │   └── ai.h/c              # Simple AI (later)
│   │
│   └── ui/                     # User interface
│       ├── ui.h/c              # UI system
│       ├── hud.h/c             # In-game HUD
│       └── menu.h/c            # Menus (later)
│
├── assets/                     # Game assets
│   ├── shaders/                # GLSL shaders
│   │   ├── basic.vert
│   │   └── basic.frag
│   ├── models/                 # 3D models (OBJ or custom)
│   ├── textures/               # Textures
│   └── data/                   # Game data (JSON/custom)
│       ├── vehicles.json
│       ├── weapons.json
│       └── arenas.json
│
├── tests/                      # Test code (later)
│
└── tools/                      # Build tools, scripts
```

---

## Core Systems

### 1. Engine Lifecycle

```c
// Simplified engine flow

int main() {
    engine_init();      // SDL, OpenGL, load assets

    while (engine_running()) {
        engine_poll_events();   // Input, window events
        engine_update();        // Game logic
        engine_render();        // Draw frame
    }

    engine_shutdown();  // Cleanup
    return 0;
}
```

### 2. Game State Machine

```
┌─────────────────────────────────────────────────────────────┐
│                      GAME STATES                            │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  ┌──────────┐     ┌──────────┐     ┌──────────────────┐    │
│  │  TITLE   │────►│  SETUP   │────►│  PLAYING         │    │
│  │  SCREEN  │     │  (pick   │     │                  │    │
│  └──────────┘     │  cars)   │     │  ┌────────────┐  │    │
│       ▲           └──────────┘     │  │ PLANNING   │  │    │
│       │                            │  └─────┬──────┘  │    │
│       │                            │        ▼         │    │
│       │                            │  ┌────────────┐  │    │
│       │                            │  │ EXECUTING  │  │    │
│       │                            │  └─────┬──────┘  │    │
│       │                            │        ▼         │    │
│       │                            │  ┌────────────┐  │    │
│       │                            │  │ RESOLVING  │  │    │
│       │           ┌──────────┐     │  └─────┬──────┘  │    │
│       └───────────│  RESULT  │◄────┴────────┘         │    │
│                   │  SCREEN  │                        │    │
│                   └──────────┘                        │    │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

### 3. Entity Structure

Using a simple struct-based approach (not full ECS for demo):

```c
// Vehicle structure
typedef struct Vehicle {
    // Identity
    uint32_t id;
    const char* name;

    // Transform
    Vec3 position;
    float rotation;     // Y-axis rotation in degrees

    // Movement
    float speed;        // Current speed (mph)
    float max_speed;
    float acceleration;
    int handling;       // Handling modifier

    // Health
    int armor[4];       // Front, Back, Left, Right
    int hull;
    int max_hull;

    // Weapons
    Weapon weapons[MAX_WEAPONS];
    int weapon_count;

    // State
    VehicleState state; // Active, Disabled, Destroyed

    // Rendering
    Mesh* mesh;
    Texture* texture;
} Vehicle;
```

### 4. Turn System

```c
typedef enum TurnPhase {
    PHASE_PLANNING,     // Players input orders
    PHASE_EXECUTE,      // Movement resolves
    PHASE_COMBAT,       // Weapons fire
    PHASE_CLEANUP       // Damage, status effects
} TurnPhase;

typedef struct TurnState {
    TurnPhase phase;
    int turn_number;

    // Orders (submitted during planning)
    VehicleOrders orders[MAX_VEHICLES];

    // Resolution queue
    // (for showing execution step-by-step)
} TurnState;

typedef struct VehicleOrders {
    uint32_t vehicle_id;

    // Movement orders
    int speed_change;   // -10 to +15
    Maneuver maneuvers[MAX_MANEUVERS_PER_TURN];
    int maneuver_count;

    // Combat orders
    WeaponTarget targets[MAX_WEAPONS];
} VehicleOrders;
```

---

## Rendering Architecture

### Pipeline Overview

```
┌─────────────────────────────────────────┐
│           RENDER PIPELINE               │
├─────────────────────────────────────────┤
│                                         │
│  1. Clear buffers                       │
│                                         │
│  2. Set camera matrices                 │
│     - View matrix (camera position)     │
│     - Projection matrix (perspective)   │
│                                         │
│  3. Render arena                        │
│     - Floor (grid visible)              │
│     - Walls                             │
│     - Obstacles                         │
│                                         │
│  4. Render vehicles                     │
│     - Sorted by depth if needed         │
│     - Apply damage/fire effects         │
│                                         │
│  5. Render effects                      │
│     - Projectiles in flight             │
│     - Explosions, smoke                 │
│                                         │
│  6. Render UI (orthographic)            │
│     - HUD elements                      │
│     - Planning interface                │
│                                         │
│  7. Swap buffers                        │
│                                         │
└─────────────────────────────────────────┘
```

### Camera System

```c
typedef struct Camera {
    Vec3 target;        // Look-at point (center of action)
    float distance;     // Distance from target
    float azimuth;      // Horizontal angle
    float elevation;    // Vertical angle

    float fov;          // Field of view
    float near_plane;
    float far_plane;

    Mat4 view_matrix;
    Mat4 projection_matrix;
} Camera;

// Orbit camera - player controls azimuth/elevation/distance
// Target can follow action or be set by player
```

---

## Data-Driven Design

### Vehicle Definitions (JSON)

```json
{
  "vehicles": [
    {
      "id": "striker",
      "name": "Striker",
      "chassis": "compact",
      "max_speed": 95,
      "acceleration": 15,
      "handling": 2,
      "armor": { "front": 10, "back": 5, "left": 5, "right": 5 },
      "hull": 20,
      "weapons": [
        { "type": "machine_gun", "mount": "front" },
        { "type": "machine_gun", "mount": "front" }
      ],
      "mesh": "models/striker.obj",
      "texture": "textures/striker.png"
    }
  ]
}
```

### Weapon Definitions (JSON)

```json
{
  "weapons": [
    {
      "id": "machine_gun",
      "name": "Machine Gun",
      "damage": 3,
      "to_hit_base": 50,
      "range_short": 10,
      "range_medium": 20,
      "range_long": 40,
      "ammo": -1,
      "fire_arc": 30
    },
    {
      "id": "rocket_launcher",
      "name": "Rocket Launcher",
      "damage": 15,
      "to_hit_base": 40,
      "range_short": 15,
      "range_medium": 30,
      "range_long": 60,
      "ammo": 6,
      "fire_arc": 15
    }
  ]
}
```

---

## Memory Management

For the demo, we'll use simple patterns:

```c
// Arena allocator for per-frame temporary data
Arena frame_arena;

// Pool allocators for fixed-size game objects
Pool vehicle_pool;
Pool projectile_pool;

// Standard malloc for long-lived resources
// (meshes, textures, loaded data)
```

---

## Build System

### CMake (Recommended)

```cmake
cmake_minimum_required(VERSION 3.16)
project(car_wars_arena C)

set(CMAKE_C_STANDARD 11)

# Find SDL3
find_package(SDL3 REQUIRED)

# Find OpenGL
find_package(OpenGL REQUIRED)

# Executable
add_executable(arena
    src/main.c
    src/core/engine.c
    src/render/renderer.c
    # ... other source files
)

target_link_libraries(arena
    SDL3::SDL3
    OpenGL::GL
    m
)
```

---

## Testing Strategy

### Debug Rendering

Built-in debug visualization:
- Grid lines
- Collision bounds
- Firing arcs
- Movement paths
- Axis indicators

Toggle with debug keys (F1, F2, etc.)

### Hot Reload (Future)

For rapid iteration:
- Shader hot reload
- Data file hot reload (vehicles, weapons)
- No code hot reload in C (requires restart)

---

## Platform Considerations

| Platform | Notes |
|----------|-------|
| **Linux (WSL2)** | Primary development platform |
| **Windows** | SDL3 + OpenGL works natively |
| **macOS** | OpenGL deprecated, but 3.3 still works |
| **Steam Deck** | Linux build should work |

---

## Dependencies

### Required
- SDL3 (window, input, audio)
- OpenGL 3.3+ headers
- C standard library

### Optional
- cglm (math library - or write our own)
- stb_image (texture loading)
- cgltf or tinyobjloader (model loading)
- cJSON (JSON parsing)

---

## Open Questions

- [ ] SDL3 vs SDL2? (SDL3 has new GPU API but is newer)
- [ ] Build own math library or use cglm?
- [ ] Model format: OBJ, glTF, or custom binary?
- [ ] How to handle shader compilation errors gracefully?
- [ ] Asset cooking pipeline needed? Or load raw files?
