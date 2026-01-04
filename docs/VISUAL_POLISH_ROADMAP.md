# Visual Polish Roadmap

## Purpose

This document outlines a strategic investment in visual polish to attract contributors to the project. While polish typically comes late in development, we face a bootstrap problem: we need contributors to build features, but we need compelling visuals to attract contributors.

**Goal:** Create a "visual vertical slice" - enough polish to produce impressive screenshots and short videos that demonstrate the project's potential.

---

## Current State

The engine has solid foundations:
- Physics-based turn execution working
- Lua scripting system functional
- Core gameplay loop implemented
- 21 power plants validated

But visuals are placeholder-level:
- Basic geometric shapes for vehicles
- Simple arena with minimal detail
- No particle effects
- No skid marks or dynamic surface effects
- No weapon visuals

---

## Strategic Approach

### The Bootstrap Problem

```
Need contributors → Need polish to attract them
Need polish → Need contributors to build it
```

**Solution:** Invest in high-impact visual features that:
1. Create compelling screenshots for README/social media
2. Produce exciting short video clips (30-60 seconds)
3. Demonstrate the physics system's potential
4. Establish the Car Wars aesthetic

### Priority Matrix

| Feature | Recruitment Impact | Effort | Priority |
|---------|-------------------|--------|----------|
| Skid marks | High | Low | **P0** |
| Tire smoke/dust | High | Medium | **P0** |
| Arena floor textures | High | Low-Medium | **P1** |
| Vehicle models | Very High | High | **P1** |
| Weapon mount visuals | Very High | Medium | **P1** |
| Fire/explosions | High | Medium | **P2** |
| Arena redesign | High | High | **P2** |
| Car Shop GUI | Medium | Very High | **Defer** |

---

## Roadmap

### Sprint 1: Quick Wins
**Goal:** Immediate visual feedback that makes gameplay feel "real"

#### Skid Marks System
- Decal-based tire tracks painted on ground during wheel slip
- Fade over time or cap at max count
- Different marks for: acceleration spin, braking, drifting
- Dark rubber marks on concrete/asphalt

#### Tire Smoke Particles
- White/grey puff particles when wheels spin or slide
- Scale intensity with slip amount
- Dust variant for dirt/gravel surfaces (future)

#### Arena Floor Upgrade
- Higher resolution concrete/asphalt texture
- Painted lane lines and boundary markings
- Oil stains and wear patterns
- Subtle grid overlay (VTT aesthetic)

#### Lighting Improvements
- Proper shadow mapping if not present
- Ambient occlusion (SSAO or baked)
- Atmospheric dust/haze for depth

**Deliverable:** 3-5 screenshots of a car drifting with skid marks and smoke

---

### Sprint 2: Hero Vehicles
**Goal:** Car Wars aesthetic - armed vehicles with personality

#### Vehicle Models (2-3 initially)
- **Armed Sedan** - classic Car Wars look, roof turret mount
- **Combat Truck** - heavier, bed-mounted weapons
- **Cycle/Trike** - fast, light, single weapon

#### Visual Requirements
- Low-poly stylized (miniature/model aesthetic)
- Clear silhouettes for team identification
- Bold color areas for team tinting
- Weathering and battle damage decals

#### Weapon Mount System
- Defined mount points: roof, hood, sides, rear
- Mount types: turret (rotating), fixed, sponson
- Visual-only initially (no gameplay integration required)

#### Weapon Models (3-4 initially)
- **Machine Gun** - belt-fed, barrel visible
- **Rocket Pod** - cluster of tubes
- **Flamethrower** - tank + nozzle
- **Recoilless Rifle** - long barrel, back-blast shield

**Deliverable:** Beauty shots of 2-3 armed vehicles from multiple angles

---

### Sprint 3: Combat Effects
**Goal:** Dynamic action moments for video capture

#### Muzzle Flash
- Sprite-based flash at weapon barrel
- Brief duration, high intensity
- Screen shake option for heavy weapons

#### Explosions
- Fireball particle burst
- Smoke trail aftermath
- Debris sprites (optional)
- Camera shake

#### Fire Effects
- Looping fire particle system
- Attach to damaged vehicles
- Smoke column rising

#### Impact Effects
- Sparks on vehicle-vehicle collision
- Sparks on vehicle-wall collision
- Dust puff on ground impact

**Deliverable:** 30-second video of combat with effects

---

### Sprint 4: Arena Redesign
**Goal:** Authentic Car Wars arena environment

#### Classic Arena Layout
Reference: Car Wars "Armadillo Autoduel Arena" style
- Rectangular main floor (200x100 units or similar)
- Concrete walls with metal reinforcement
- Entry/exit gates on short ends
- Central obstacles (pillars, barriers)
- Ramps for jumps/elevation

#### Arena Elements
- **Walls** - Concrete with tire marks, damage decals
- **Barriers** - Jersey barriers, concrete blocks
- **Pillars** - Structural columns as obstacles
- **Ramps** - Jump ramps, banked curves
- **Gates** - Roll-up doors, starting positions

#### Spectator Elements (Low Priority)
- Simple stands geometry
- Crowd noise (audio, later)
- Announcer booth

#### Multiple Arenas (Future)
- "The Pit" - basic starter arena
- "Figure 8" - crossing paths
- "The Maze" - tight corridors
- Outdoor variants

**Deliverable:** Flythrough video of detailed arena

---

### Deferred: Car Shop GUI
**Rationale:** High effort, doesn't photograph well, better built with contributor help

When implemented:
- Chassis selection
- Power plant selection
- Weapon loadout
- Armor allocation
- Paint/decal customization
- Stats summary
- Save/load configurations

---

## Asset Strategy

### Blender + Python Pipeline (Recommended)

The modular nature of Car Wars vehicles (base vehicle + weapon loadouts) maps perfectly to a scripted assembly pipeline.

#### Why This Approach

| Problem with Purchased Assets | Blender Pipeline Solution |
|------------------------------|---------------------------|
| Armed vehicles are rare | Assemble weapons onto base cars |
| Weapon placement never right | Define exact mount points |
| Art styles clash | Consistent processing |
| Need many variants | Script generates combinations |

#### Pipeline Architecture

```
┌─────────────────────────────────────────────────────────┐
│                    BLENDER PIPELINE                      │
├─────────────────────────────────────────────────────────┤
│                                                          │
│  BASE VEHICLES          WEAPONS           OUTPUT         │
│  ├── sedan.blend        ├── machinegun    ├── Combos    │
│  ├── truck.blend    +   ├── rocket_pod  → │  exported   │
│  └── cycle.blend        ├── flamethrower  │  to glTF    │
│                         └── recoilless    │             │
│                                                          │
│  Mount points defined as Blender empties:               │
│  - mount_roof_turret                                    │
│  - mount_hood_fixed                                     │
│  - mount_left_sponson                                   │
│  - mount_right_sponson                                  │
│                                                          │
│  Python script:                                         │
│  1. Loads base vehicle                                  │
│  2. Attaches weapons to specified mounts                │
│  3. Applies team colors/materials                       │
│  4. Exports game-ready glTF                            │
│                                                          │
└─────────────────────────────────────────────────────────┘
```

#### Combination Math
- 3 base vehicles × 5 weapons × 4 mount positions = 60+ unique configs
- All generated automatically from a small set of source models

#### Implementation Phases

1. **Proof of Concept** - One car + one weapon + export script
2. **Build Library** - 3 vehicles, 5 weapons, standard mounts
3. **Integration** - Connect to Car Shop GUI or pre-generate all combos

### Vehicle Model Sources

#### Free / CC0 Sources
| Source | URL | Notes |
|--------|-----|-------|
| **Kenney** | kenney.nl/assets | Game-ready, CC0, consistent style |
| **Quaternius** | quaternius.com | Low-poly packs, free |
| **OpenGameArt** | opengameart.org | Search "low poly car" |
| **Sketchfab** | sketchfab.com (filter CC0) | Huge variety, quality varies |
| **Poly Pizza** | poly.pizza | Curated low-poly, CC0 |

#### Paid (Budget Options)
| Source | URL | Price Range |
|--------|-----|-------------|
| **TurboSquid** | turbosquid.com | $10-50 per model |
| **CGTrader** | cgtrader.com | $5-30 per model |
| **Sketchfab Store** | sketchfab.com/store | $5-20 per model |

#### Quick Placeholder Strategy
Get ANY decent low-poly car immediately to replace primitives. Even a generic sedan dramatically improves screenshots. Weapon mounts come later via Blender pipeline.

**Direct Links to Browse:**
| Source | Direct Link | Notes |
|--------|-------------|-------|
| Kenney Car Kit | https://kenney.nl/assets/car-kit | Modular parts, perfect for kitbash |
| Kenney Vehicles | https://kenney.nl/assets?q=vehicle | Various vehicle packs |
| Quaternius Cars | https://quaternius.com/packs/ultimatecars.html | Low-poly car pack |
| Poly Pizza Cars | https://poly.pizza/search/car | Search results for "car" |
| Sketchfab CC0 | https://sketchfab.com/search?features=downloadable&licenses=7c23a1ba438d4306920229c12afcb5f9&q=low+poly+car&type=models | Pre-filtered CC0 |

### AI-Generated 3D Models

AI 3D generation has become viable for game assets, especially stylized/low-poly content.

#### Available Services

| Service | URL | Best For | Pricing |
|---------|-----|----------|---------|
| **Meshy.ai** | https://meshy.ai | Text-to-3D, Image-to-3D | Free tier, $20/mo pro |
| **Tripo3D** | https://tripo3d.ai | Fast generation | Free tier available |
| **Rodin (Hyperhuman)** | https://hyper3d.ai | High quality | Credits-based |
| **Luma Genie** | https://lumalabs.ai/genie | Quick concepts | Free to try |
| **CSM** | https://csm.ai | Image-to-3D | Free tier |
| **Spline AI** | https://spline.design | Web-based editor | Free tier |

#### What Works Well
- Simple/stylized objects (weapons, props, basic vehicles)
- Quick concept exploration
- Low-poly aesthetic (hides mesh issues)

#### What Needs Work
- Game-ready topology (often messy, needs cleanup)
- Consistency across multiple generations
- UV mapping (usually needs redo)

#### Recommended AI Workflow
1. Generate base model with text prompt (e.g., "low poly muscle car, post apocalyptic")
2. Import to Blender
3. Retopology if needed (remesh modifier or manual)
4. Fix UVs and materials
5. Export to game format

#### AI Viability by Asset Type
| Asset Type | AI Quality | Notes |
|------------|------------|-------|
| Weapons | Good | Simple geometry, works well |
| Props (barrels, barriers) | Excellent | Simple shapes |
| Arena structures | Good | Walls, ramps, pillars |
| Base vehicles | Medium | Often needs topology cleanup |
| Detailed vehicles | Poor | Too complex, better to kitbash |

### Texture and Effect Assets

#### Textures
- **Poly Haven** - CC0 PBR textures (concrete, metal, asphalt)
- **ambientCG** - CC0 materials
- Create custom decals (skid marks, oil stains, damage)

#### Particles
- Build custom particle systems
- Reference: Unity/Unreal effect tutorials for approach
- Keep stylized, not photorealistic

---

## Success Criteria

### Screenshot Test
Can we produce 5 images that would make someone say "I want to play this"?

1. Hero vehicle beauty shot (armed car, dramatic angle)
2. Drift action shot (skid marks, tire smoke, motion blur)
3. Arena overview (detailed environment, multiple vehicles)
4. Combat moment (muzzle flash, explosion, chaos)
5. Turn planning UI (showing tactical depth)

### Video Test
Can we produce a 60-second video that demonstrates:

1. Vehicle selection/customization (even if minimal)
2. Turn planning interface
3. Physics execution with visual feedback
4. Combat effects (weapons, explosions)
5. Dramatic moment (crash, near miss, kill)

### Recruitment Test
Does the visual package answer:
- "What kind of game is this?" - Clear Car Wars/vehicular combat identity
- "Is this a real project?" - Professional-looking, not programmer art
- "What would I work on?" - Obvious areas for contribution

---

## Open Questions

- [ ] Budget for commissioned assets?
- [ ] Target poly count for vehicles? (mobile-friendly vs desktop)
- [ ] PBR vs stylized shading approach?
- [ ] Particle system: CPU or GPU-based?
- [ ] Decal system: projected vs mesh-based?

---

## References

### Visual Style
- Car Wars (Steve Jackson Games) - original tabletop art
- Gaslands (Osprey Games) - miniature game aesthetic
- Interstate '76 - classic vehicular combat game
- Twisted Metal (stylized, not realistic)

### Technical
- [docs/vision.md](vision.md) - Visual identity guidelines
- [docs/ROADMAP.md](ROADMAP.md) - Overall project roadmap
- [docs/CONFIG_GUIDE.md](CONFIG_GUIDE.md) - Vehicle configuration

---

## Revision History

| Date | Changes |
|------|---------|
| 2025-01-03 | Initial draft - visual polish strategy |
