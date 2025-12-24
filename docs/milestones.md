# Milestones: Build Order

## Philosophy

Each milestone should produce something **runnable and visible**. No milestone should be "just infrastructure" - every step should show progress on screen.

**Two-Phase Approach:**
1. **Phase 1: Level Editor** - Build rendering, validate the visual aesthetic
2. **Phase 2: Game** - Add game logic on top of proven rendering

This lets us answer "Does this look like Car Wars?" before we tackle "Is this fun to play?"

---

# PHASE 1: LEVEL EDITOR

The Level Editor is a standalone tool that:
- Validates rendering pipeline and art direction
- Becomes useful for creating arenas later
- Shares code with the game (renderer, assets, math)

---

## Milestone 0: Foundation (DONE)

**Goal:** Prove the development environment works.

- [x] WSL2 with GPU passthrough verified
- [x] OpenGL hardware acceleration confirmed
- [x] Test program renders 3D (spinning cube)
- [x] Build system works (Makefile)

**Deliverable:** `3d-hardware-test/` with working cube demo.

---

## Milestone E1: SDL2 Window + Fly Camera (DONE)

**Goal:** A window we can fly around in.

- [x] SDL2 window opens with OpenGL 3.3+ context (using 4.6 via D3D12)
- [x] Clear color visible (dark grey)
- [x] Frame timing (delta time, FPS display in title bar)
- [x] Fly camera: WASD + mouse look (right-click to capture)
- [x] Shift to move faster, scroll to adjust speed
- [x] ESC to quit, F11 fullscreen

**Deliverable:** `arena-editor/` with working fly camera and grid visualization.

**Note:** SDL3 failed to build; using SDL2 with platform abstraction for future portability. WSL2 requires `GALLIUM_DRIVER=d3d12` for GPU acceleration (handled by `run.sh`).

---

## Milestone E2: Arena Floor + Grid (DONE)

**Goal:** Something that looks like a game board.

- [x] Flat plane rendered (200x200 unit arena floor)
- [x] Procedural grid shader (no texture files needed)
- [x] Grid aligned to Car Wars scale (1 unit = 1 inch, major lines every 5)
- [x] Floor appearance: concrete feel with noise variation
- [x] Axis indicators (red=X, blue=Z on floor, green=Y vertical)
- [x] Distance fog for depth perception

**Deliverable:** Shader-based floor with procedural concrete texture and grid overlay.

---

## Milestone E3: Arena Walls + Obstacles (DONE)

**Goal:** An enclosed space with things in it.

- [x] Box mesh renderer with VAO/VBO and lighting shader
- [x] Arena walls form 60x60 unit perimeter
- [x] Obstacles: central cross, corner pillars, crates, barrier walls
- [x] Directional lighting (sun from upper-right)
- [ ] Shadows (deferred to later)

**Deliverable:** Enclosed lit arena with obstacles at 1920x1080.

---

## Milestone E4: Mesh + Texture Loading

**Goal:** Load real assets, not just primitives.

- [ ] OBJ or glTF mesh loader
- [ ] PNG texture loader
- [ ] Material system (diffuse texture + color)
- [ ] Load placeholder vehicle mesh
- [ ] Load placeholder wall/obstacle textures

**Deliverable:** Real textured meshes in the scene.

---

## Milestone E5: Object Placement

**Goal:** Interactively build the arena.

- [ ] Object palette (walls, obstacles, vehicles)
- [ ] Click to place object at grid position
- [ ] Click to select existing object
- [ ] Move selected object (drag or arrow keys)
- [ ] Delete selected object
- [ ] Rotate selected object (R key or UI)

**Deliverable:** Can populate an arena with objects.

---

## Milestone E6: Save/Load Arenas

**Goal:** Persist your work.

- [ ] Arena file format (JSON)
- [ ] Save current arena to file
- [ ] Load arena from file
- [ ] New/clear arena
- [ ] Simple file browser or fixed save slots

**Deliverable:** Build an arena, save it, reload it later.

---

## Milestone E7: Visual Polish

**Goal:** Does this look like Car Wars?

- [ ] Test AI-generated textures from book references
- [ ] Tweak lighting for mood
- [ ] Add skybox or background gradient
- [ ] Screenshot tool (F12)
- [ ] Compare screenshots to book art

**Deliverable:** Screenshots that evoke the Car Wars aesthetic.

---

## Milestone E8: Editor UI

**Goal:** Usable tool, not just dev hacks.

- [ ] On-screen toolbar/palette
- [ ] Property panel for selected object
- [ ] Status bar (position, object count, etc.)
- [ ] Help overlay (show controls)
- [ ] Editor vs. Preview mode toggle

**Deliverable:** A real level editor someone else could use.

---

# PHASE 2: GAME

With rendering proven and arenas buildable, we add game logic.

---

## Milestone G1: Game Window + Arena Loading

**Goal:** Game loads and displays an arena.

- [ ] Separate game executable (shares code with editor)
- [ ] Load arena JSON file
- [ ] Display arena in game view
- [ ] Game camera (orbit around arena, not fly)
- [ ] Vehicles loaded at spawn positions

**Deliverable:** Game displays a built arena with vehicles.

---

## Milestone G2: Turn State Machine

**Goal:** Basic turn flow works.

- [ ] Game states: PLANNING → EXECUTING → RESOLVING → repeat
- [ ] UI shows current phase
- [ ] "End Planning" button to advance
- [ ] Turn counter display

**Deliverable:** Can cycle through empty turn phases.

---

## Milestone G3: Movement Planning

**Goal:** Player can give orders.

- [ ] Click vehicle to select
- [ ] Planning UI: speed control (accel/decel/maintain)
- [ ] Planning UI: maneuver selection (straight/turn)
- [ ] Movement preview (ghost line showing path)
- [ ] Commit orders button

**Deliverable:** Can plan movement for a vehicle.

---

## Milestone G4: Movement Execution

**Goal:** Vehicle moves according to orders.

- [ ] Execute phase animates movement
- [ ] Vehicle follows planned path
- [ ] Collision with walls (stop, maybe damage)
- [ ] Camera tracks action
- [ ] Return to planning when done

**Deliverable:** Vehicle drives around the arena.

---

## Milestone G5: NPC Opponent

**Goal:** Someone to fight.

- [ ] NPC controller (AI plans during planning phase)
- [ ] NPC logic: move toward player
- [ ] NPC logic: maintain combat distance
- [ ] Both vehicles move simultaneously in execution
- [ ] Vehicle-to-vehicle collision (ramming)

**Deliverable:** Player vs NPC movement duel.

---

## Milestone G6: Combat System

**Goal:** Cars can shoot.

- [ ] Weapon targeting in planning phase
- [ ] Firing arc visualization
- [ ] Combat resolution (hit roll, damage roll)
- [ ] Dice visualization during resolution
- [ ] Damage applied (armor → hull)
- [ ] Visual feedback (sparks, smoke)

**Deliverable:** Vehicles can shoot each other.

---

## Milestone G7: Win Condition + Game Loop

**Goal:** It's a complete game.

- [ ] Victory when enemy hull = 0
- [ ] Victory/defeat screen
- [ ] Play again option
- [ ] Title screen
- [ ] Arena selection (load different arenas)

**Deliverable:** Playable arena duel from start to finish.

---

## Milestone G8: Polish Pass

**Goal:** It feels good.

- [ ] Explosion when vehicle destroyed
- [ ] Fire on damaged vehicles
- [ ] Sound effects (engine, guns, explosions)
- [ ] UI styling (record sheet aesthetic)
- [ ] Camera polish (smooth transitions)

**Deliverable:** Something you'd show to a Car Wars fan.

---

## Milestone G9: AI Improvements

**Goal:** Smarter opponent.

- [ ] AI avoids walls
- [ ] AI uses cover
- [ ] AI varies tactics
- [ ] Difficulty settings

**Deliverable:** A challenging fight.

---

# FUTURE MILESTONES (Post-Demo)

| Milestone | Description |
|-----------|-------------|
| **Vehicle Builder** | Choose chassis, weapons, armor |
| **Multiple Arenas** | Different layouts, terrain types |
| **Full Damage Model** | Components, crew, cargo |
| **Saved Games** | Save/load mid-game |
| **Replays** | Watch past games |
| **Networking** | Online multiplayer |
| **Campaign** | Story mode, progression |
| **Steam Integration** | Achievements, workshop |

---

## Risk Areas

| Risk | Mitigation |
|------|------------|
| **Art direction** unclear | Phase 1 validates look before game logic |
| **Simultaneous movement** complex | Start simple, polish later |
| **Scope creep** | Strict milestone focus |
| **SDL3 is new** | Fallback to SDL2 if needed |
| **Textures don't match vision** | Iterate in E7 before moving on |

---

## Definition of "Phase 1 Complete"

The Level Editor is complete when:

1. Can fly through a 3D arena
2. Can place walls, obstacles, vehicles
3. Can save/load arena layouts
4. Textures evoke Car Wars book art
5. Screenshots look promising

---

## Definition of "Demo Complete"

The game demo is complete when:

1. Two vehicles can fight in an arena
2. Movement feels correct (speed, turning)
3. Weapons work (target, fire, damage)
4. Someone wins
5. You can restart and play again
6. A Car Wars fan says "this feels right"
