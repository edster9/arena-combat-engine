# Assets: Art, Models, and Audio

## Philosophy

Assets should be developed in parallel with code, but **placeholders are acceptable** to keep momentum. A working game with cubes is better than a stalled project waiting for art.

---

## Asset Pipeline

```
┌─────────────────────────────────────────────────────────────┐
│                     ASSET WORKFLOW                          │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  SOURCE FILES          EXPORT              IN-GAME          │
│  ────────────          ──────              ───────          │
│                                                             │
│  Blender (.blend)  ──► OBJ/glTF ─────────► Mesh             │
│                                                             │
│  Photoshop/GIMP    ──► PNG (power of 2) ─► Texture          │
│  (.psd/.xcf)                                                │
│                                                             │
│  Audacity          ──► WAV/OGG ──────────► Sound            │
│  (.aup)                                                     │
│                                                             │
│  Hand-written      ──► GLSL ─────────────► Shader           │
│  (.vert, .frag)                                             │
│                                                             │
│  JSON files        ──► JSON ─────────────► Game Data        │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

---

## 3D Models Required

### Phase 1: Demo

| Asset                | Description             | Placeholder     |
| -------------------- | ----------------------- | --------------- |
| **Vehicle: Striker** | Compact car, aggressive | Colored box     |
| **Vehicle: Bruiser** | Sedan, bulky, armored   | Colored box     |
| **Arena Floor**      | Flat plane with grid    | Plane primitive |
| **Arena Wall**       | Concrete/metal barrier  | Box primitive   |
| **Obstacle: Block**  | Concrete barrier        | Box primitive   |

### Phase 2: Polish

| Asset                   | Description                     |
| ----------------------- | ------------------------------- |
| **Weapon: Machine Gun** | Mounted gun barrels             |
| **Weapon: Rocket Pod**  | Tube launcher                   |
| **Debris**              | Scattered parts for destruction |
| **Wreck**               | Destroyed vehicle husk          |

### Model Specifications

```
Polycount targets (per vehicle):
  - Placeholder: 12 triangles (box)
  - Low-poly: 500-1000 triangles
  - Final: 2000-5000 triangles

Coordinate system:
  - Y-up
  - Front of vehicle faces +Z
  - Origin at vehicle center, on ground plane

Scale:
  - 1 unit = 1 inch (Car Wars scale)
  - Typical car: ~180 units long (15 feet)
```

---

## Textures Required

### Phase 1: Demo

| Asset                | Size    | Description              |
| -------------------- | ------- | ------------------------ |
| **Arena floor**      | 512x512 | Concrete with grid lines |
| **Arena wall**       | 256x256 | Metal/concrete barrier   |
| **Vehicle: Striker** | 256x256 | Red/orange team color    |
| **Vehicle: Bruiser** | 256x256 | Blue/green team color    |

### Phase 2: Polish

| Asset              | Size    | Description                  |
| ------------------ | ------- | ---------------------------- |
| **Damage overlay** | 256x256 | Scratches, dents, burns      |
| **Fire texture**   | 128x128 | Animated fire (sprite sheet) |
| **Smoke texture**  | 128x128 | Particle smoke               |
| **Explosion**      | 256x256 | Animated explosion           |
| **Muzzle flash**   | 64x64   | Gun firing effect            |
| **Tracer**         | 32x32   | Bullet trail                 |

### Texture Guidelines

```
Format: PNG (RGBA)
Size: Power of 2 (64, 128, 256, 512, 1024)
Style: Painterly/illustrated, not photo-realistic
Mip-maps: Generated at load time
```

---

## Shaders Required

### Phase 1: Demo

| Shader                    | Purpose                       |
| ------------------------- | ----------------------------- |
| **basic.vert/frag**       | Simple textured + lit         |
| **grid.vert/frag**        | Arena floor with grid overlay |
| **solid_color.vert/frag** | Debug/placeholder shapes      |

### Phase 2: Polish

| Shader                 | Purpose                    |
| ---------------------- | -------------------------- |
| **vehicle.vert/frag**  | Vehicle with damage states |
| **fire.vert/frag**     | Animated fire effect       |
| **particle.vert/frag** | Particle systems           |
| **ui.vert/frag**       | 2D UI elements             |

---

## Audio Required

### Phase 1: Demo (Optional)

Audio not required for core demo but nice to have:

| Sound           | Description      |
| --------------- | ---------------- |
| **Engine idle** | Vehicle humming  |
| **Engine rev**  | Acceleration     |
| **Collision**   | Metal crash      |
| **Machine gun** | Rapid fire burst |

### Phase 2: Polish

| Sound               | Description       |
| ------------------- | ----------------- |
| **Rocket launch**   | Whoosh            |
| **Rocket hit**      | Explosion         |
| **Tire screech**    | Hard turning      |
| **Fire crackle**    | Burning vehicle   |
| **Victory fanfare** | Game won          |
| **Ambient**         | Arena crowd noise |

### Audio Specifications

```
Format: WAV (16-bit, 44.1kHz) or OGG Vorbis
Mono for 3D sounds, Stereo for music/UI
Normalize to -3dB peak
```

---

## UI Assets Required

### Phase 1: Demo

| Asset          | Description                   |
| -------------- | ----------------------------- |
| **Button**     | 9-slice button graphic        |
| **Panel**      | UI background panel           |
| **Font**       | Monospace or typewriter style |
| **Health bar** | Armor/hull display            |

### Phase 2: Polish

| Asset                    | Description            |
| ------------------------ | ---------------------- |
| **Vehicle record sheet** | Classic Car Wars style |
| **Weapon icons**         | MG, rocket, etc.       |
| **Status icons**         | Fire, disabled, etc.   |
| **Turn indicator**       | Current phase display  |

### UI Style Direction

```
Aesthetic: Paper/cardboard, typewriter text
Colors: Tan, brown, black ink
Borders: Rough edges, stamped look
Reference: Original Car Wars record sheets
```

---

## Placeholder Strategy

While developing, use programmer art:

| Final Asset   | Placeholder                       |
| ------------- | --------------------------------- |
| Vehicle model | Colored box with arrow for facing |
| Arena floor   | Checkered or solid color plane    |
| Weapons       | Cylinder primitives               |
| Fire/smoke    | Colored billboards                |
| UI buttons    | Solid colored rectangles          |

**Rule:** If a placeholder is blocking progress, simplify until you can move forward.

---

## Asset Sources

### Create Ourselves

- Shaders (must be custom)
- Game data (JSON configs)
- UI layout

### Commission or Purchase

- 3D vehicle models (stylized)
- Texture art (stylized)
- Sound effects

### Free/CC Resources (Temporary)

| Source          | Type                     |
| --------------- | ------------------------ |
| OpenGameArt.org | Models, textures, sounds |
| Freesound.org   | Sound effects            |
| Kenney.nl       | Game assets, CC0         |
| Quaternius      | Low-poly 3D models       |

**Note:** Check licenses. Prefer CC0 or CC-BY for commercial use.

---

## Art Direction Reference

To communicate the desired style to artists:

### Vehicle Style

- Chunky, solid, weighted
- Visible weapons (not hidden)
- Bright team colors
- Wear and tear details
- NOT realistic, NOT cartoony

### Arena Style

- Industrial, concrete
- Grid visible but integrated
- Stains, cracks, history
- Barriers feel heavy
- Lighting: harsh overhead spots

### Reference Images to Collect

- [ ] Original Car Wars box art
- [ ] Denis Loubet vehicle illustrations
- [ ] Gaslands miniatures (modern reference)
- [ ] Micro Machines toys (toy car aesthetic)
- [ ] Tabletop Simulator board games

---

## Art Creation Strategy (DECISION)

### Approach: AI-Assisted + Purchased Assets

| Asset Type                | Strategy                                               |
| ------------------------- | ------------------------------------------------------ |
| **Textures**              | AI-generated from book art references, then refined    |
| **Vehicle models**        | Purchase/license stylized car models, modify as needed |
| **Building/arena models** | Purchase basic industrial assets, customize            |
| **UI elements**           | AI-assisted based on record sheet scans                |
| **Shaders**               | Hand-written (must be custom)                          |
| **Game data**             | Hand-written JSON                                      |

### Reference Material

Extract from the Car Wars books in `/books/`:

- Vehicle illustrations for AI texture prompts
- Arena layouts for level design
- Record sheets for UI design
- Uncle Albert's catalog art for equipment icons

### AI Texture Workflow

1. Scan/screenshot reference from books
2. Describe style to AI image generator
3. Generate base texture
4. Clean up in image editor (GIMP/Photoshop)
5. Ensure seamless tiling if needed
6. Export as PNG power-of-2

### Model Purchasing

Potential sources:

- **Sketchfab** - Stylized vehicle models
- **TurboSquid** - Industrial environments
- **CGTrader** - Various game-ready assets
- **Unity Asset Store** - Even for non-Unity use (check license)

Budget: Allocate for 2-3 quality vehicle models and basic arena kit.

---

## 3D Model Resources Research

### Currently Using

#### Kenney Car Kit (CC0) - INSTALLED

- **Location:** `assets/models/vehicles/kenney-car-kit/`
- **License:** CC0 (Public Domain) - Free for commercial use
- **Format:** OBJ, FBX, GLB (all formats included)
- **Website:** https://kenney.nl/assets/car-kit

**Available Models:**
| Vehicle Type | Models |
|--------------|--------|
| Cars | sedan, sedan-sports, hatchback-sports, taxi, police |
| Racing | race, race-future |
| Trucks | truck, truck-flat, delivery, delivery-flat |
| SUVs | suv, suv-luxury |
| Utility | ambulance, firetruck, garbage-truck, van |
| Tractors | tractor, tractor-police, tractor-shovel |

**Bonus Assets:**

- Multiple wheel variants (default, dark, racing, truck)
- Debris pieces (doors, tires, plates, spoilers)
- Traffic props (cones, boxes)
- Textures included

**Recommended for Car Wars:**

- `sedan-sports` - Good compact armed vehicle base
- `race-future` - Futuristic variant
- `truck` / `truck-flat` - Heavy vehicle base
- `suv` - Mid-size armored vehicle base
- Debris pieces for destruction effects

---

### Alternative Sources (For Future Consideration)

#### Free Resources (CC0/CC-BY)

| Source                | Type        | Notes                       |
| --------------------- | ----------- | --------------------------- |
| **OpenGameArt.org**   | Various     | Check individual licenses   |
| **Quaternius**        | Low-poly 3D | Free packs, stylized        |
| **Sketchfab (Free)**  | Mixed       | Filter by license carefully |
| **Turbosquid (Free)** | Mixed       | Check commercial use rights |

#### Paid Resources (Higher Quality)

| Source                | Type              | Price Range | Notes                        |
| --------------------- | ----------------- | ----------- | ---------------------------- |
| **Sketchfab Store**   | Stylized vehicles | $5-50       | Good low-poly options        |
| **CGTrader**          | Game-ready assets | $10-100     | Wide variety                 |
| **Unity Asset Store** | Vehicle packs     | $15-75      | License allows non-Unity use |
| **TurboSquid Pro**    | Professional      | $20-200     | High quality, check license  |

#### Gaslands/Miniature Style

For authentic Car Wars miniature aesthetic:

- **Thingiverse** - Gaslands STL files (need 3D printing or conversion)
- **MyMiniFactory** - Combat vehicles STLs
- **Patreon creators** - Custom vehicle designs

**Note:** STL files require conversion to usable game formats (OBJ/FBX/glTF).

---

### Recommended Progression

1. **Phase 1 (Now):** Kenney Car Kit for prototyping and scale testing
2. **Phase 2 (Demo):** Modify Kenney models with weapon attachments in Blender
3. **Phase 3 (Polish):** Commission or purchase stylized combat vehicles
4. **Phase 4 (Release):** Original models matching Car Wars aesthetic

---

## Open Questions

- [ ] What modeling software? Blender is free and capable.
- [ ] Style guide document needed for consistency?
- [ ] Vehicle modularity: One mesh or separate parts (weapons, wheels)?
- [ ] Which AI image generator? (Midjourney, Stable Diffusion, DALL-E)
