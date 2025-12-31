# Car Wars 6th Edition vs Classic (Compendium) Comparison

**Date:** 2025-12-30
**Purpose:** Analyze differences between Car Wars 6th Edition and Classic rules to inform Arena Combat Engine development

## Executive Summary

Car Wars 6th Edition is a **complete redesign**, not an update. It shares the high concept (armed vehicular combat) but fundamentally changes the turn structure, vehicle construction, and combat resolution. The classic Compendium rules remain far more suitable for a faithful tabletop adaptation.

---

## Turn-Based System Comparison

### Classic Car Wars (Compendium 2nd Edition)

**5-Phase System:**
- Each turn = 1 second, divided into **5 phases** (0.2 seconds each)
- Movement determined by **Movement Chart** cross-referencing speed
- Vehicles move in specific phases based on speed:
  - 10 mph: Phase 1 only (1" move)
  - 20 mph: Phases 1, 3 (1" each)
  - 30 mph: Phases 1, 3, 5 (1" each)
  - 40 mph: Phases 1, 2, 3, 5 (1" each)
  - 50 mph: All 5 phases (1" each)
  - 60+ mph: Some phases have 2" moves
- **"Odd" speeds** (5, 15, 25 mph) include a half-move phase
- Speed changes happen **once per turn**, at the beginning of a phase
- Faster vehicles move first within a phase

**Key Mechanics:**
- Handling Status (HS) tracking - starts at Handling Class, decreases with maneuvers
- Control Table - cross-reference HS with speed to determine control roll needed
- Crash Tables (1-6) for different loss-of-control scenarios
- Difficulty ratings D0-D7+ for maneuvers
- HS recovers at end of each turn (by HC value)

### 6th Edition

**Single-Phase System:**
- Each round: Check Speed → Take Control → Manage Fires → Movement → Combat → End
- **One movement phase per turn** - all vehicles move once
- Movement points = speed setting (1-5)
- Each movement point = 1 car-length using turning key
- Players activate in order (first player clockwise)

**Key Mechanics:**
- Control tokens (spend to avoid losing control)
- Ace tokens (earned by maneuvers, spent for defense re-rolls)
- No Handling Status - just binary "in control" or "out of control"
- Simplified maneuvers: Drive Straight, Slight Turn (free), Slide, Turn (cost dice)
- Difficulty zones D1-D4 on turning key (no D5-D7)
- Spin-out severity based on unpaid control costs

### Impact on Your Engine

| Feature | Classic (your target) | 6th Edition |
|---------|----------------------|-------------|
| Phases per turn | 5 | 1 |
| Movement timing | Speed-based phase chart | Sequential activation |
| Handling system | HS tracks cumulative risk | Binary control tokens |
| Difficulty range | D0-D7+ | D1-D4 |
| Simultaneous planning | Yes (declare before execute) | No (sequential) |
| Speed granularity | 5 mph increments | 1-5 abstract levels |

**Recommendation:** Your current 5-phase implementation is correct for classic rules. 6th Edition's single-phase system loses the tactical depth of speed-based phase timing.

---

## Equipment and Vehicle Construction

### Classic Compendium - Detailed Construction

**Chassis Types (Cars):**
| Body | Cost | Weight | Max Load | Spaces | Armor Cost/Weight |
|------|------|--------|----------|--------|-------------------|
| Subcompact | $400 | 800 | 2,400 | 5 | $15/3 |
| Compact | $600 | 1,000 | 3,200 | 8 | $16/4 |
| Mid-size | $800 | 1,200 | 4,200 | 11 | $18/5 |
| Sedan | $1,000 | 1,500 | 5,000 | 14 | $19/5 |
| Luxury | $1,500 | 2,000 | 6,000 | 18 | $20/5 |
| Station Wagon | $1,800 | 2,200 | 6,800 | 22 | $20/6 |
| Pickup | $2,500 | 3,200 | 7,500 | 25 | $22/7 |
| Van | $3,000 | 3,500 | 8,000 | 30 | $23/7 |
| Camper | $3,500 | 4,000 | 9,000 | 35 | $24/8 |

**Chassis Modifications:**
- Extra Heavy: +$1,000, x1.5 max load, -1 space
- Streamlined: +$500, +5 mph top speed
- Reinforced: +50% chassis cost, takes 50% less collision damage

**Suspension Types:**
| Suspension | Cost | HC |
|------------|------|-----|
| Light | $200 | 1 |
| Standard | $400 | 2 |
| Heavy | $600 | 3 |
| Off-Road | $1,000 | 2 (negates off-road penalty) |
| Active | $2,000 | 4 |

**Power Plants (Electric):**
| Plant | Cost | Weight | Spaces | DP | Power Factors |
|-------|------|--------|--------|----|---------------|
| Small | $500 | 500 | 3 | 5 | 800 |
| Medium | $1,000 | 700 | 4 | 8 | 1,400 |
| Large | $2,000 | 900 | 5 | 10 | 2,000 |
| Super | $3,000 | 1,100 | 6 | 12 | 2,600 |
| Sport | $6,000 | 1,000 | 6 | 12 | 3,000 |
| Thundercat | $12,000 | 2,000 | 8 | 15 | 6,700 |

**Power Plant Accessories:**
- Platinum Catalysts: +20% cost, +5% power factors
- Superconductors: +50% cost, +10% power factors
- Extra Power Cells: +25% cost/weight, +50% power units
- ISC (Improved Supercharger Capacitors): $500, +5 acc/+20 top for 3 turns

**Gas Engines (alternative):**
| Engine | Cost | Weight | Spaces | DP | Power | Base MPG |
|--------|------|--------|--------|----|-------|----------|
| 10 cid | $400 | 60 | 1 | 1 | 300 | 80 |
| 50 cid | $1,250 | 150 | 1 | 3 | 700 | 60 |
| 100 cid | $2,500 | 265 | 2 | 6 | 1,300 | 50 |
| 200 cid | $5,500 | 480 | 4 | 12 | 2,500 | 35 |
| 350 cid | $9,500 | 975 | 7 | 19 | 5,000 | 18 |
| 500 cid | $13,000 | 1,200 | 10 | 26 | 9,500 | 12 |

**Acceleration/Top Speed Formulas:**
- Acceleration: 5/10/15 mph based on power factors vs weight ratio
  - < 1/3 weight: won't move
  - 1/3 to 1/2: 5 mph acceleration
  - 1/2 to 1x: 10 mph acceleration
  - 1x+: 15 mph acceleration
- Top Speed (electric): 360 × PF / (PF + Weight)
- Top Speed (gas): 240 × PF / (PF + Weight)

**Tires:**
| Type | Cost | Weight | DP |
|------|------|--------|-----|
| Standard | $50 | 30 | 4 |
| Heavy-Duty | $100 | 40 | 6 |
| Puncture-Resistant | $200 | 50 | 9 |
| Solid | $500 | 75 | 12 |
| Plasticore | $1,000 | 150 | 25 |

**Tire Modifications:**
- Steelbelting: +50% cost/weight, +25% DP
- Radial: +150% cost, +20% weight, -1 DP, +1 HC
- Off-Road: +20% cost, +5 lbs, +1 HC off-road
- Racing Slick: +300% cost, +100% weight, +1 DP, +2 HC (penalties on oil/ice)
- Fireproofing: 2x cost

### 6th Edition - Simplified Cards

**No Chassis Types:**
- All cars use the same base (the plastic miniature)
- No frame sizes, no weight calculations
- No max load or space tracking

**Build Point System:**
- Agree on build total (e.g., 16 BP small game, 36 BP large game)
- Purchase cards from deck:
  - Weapons (various costs, 6+ BP weapons restricted in small games)
  - Accessories (modify rules)
  - Upgrades (attach to crew area)
  - Structure (defensive bonuses per side)

**Crew Point System:**
- Hire 1 driver + 1 gunner
- Purchase gear and sidearms
- Everyone starts with Hand Cannon sidearm

**No Power Plants:**
- No engine selection
- Speed is 0-5 abstract levels
- No acceleration calculation
- No range/fuel management

**Example 6th Ed Weapons:**
- Heavy Machine Gun: 3 BP, 1-2 damage, yellow dice
- Autocannon: Higher cost, more damage
- Weapons are cards placed adjacent to dashboard

### Impact on Your Engine

| Feature | Classic | 6th Edition |
|---------|---------|-------------|
| Vehicle variety | 9+ body types | 1 (miniature) |
| Chassis customization | Weight, spaces, mods | None |
| Power plant selection | 6 electric + 13 gas | None |
| Engine accessories | 5+ types | None |
| Tire types | 5 types + 5 mods | None (abstracted) |
| Suspension | 5 types | None |
| Armor placement | 6 locations, variable points | 4 sliders, fixed values |
| Weight tracking | Yes (affects acceleration) | No |
| Space management | Yes (limited by body) | No |
| Cost management | Yes (budget builds) | Build points only |

**Recommendation:** Your engine should implement the full classic construction system. 6th Edition's card-based system is incompatible with detailed vehicle design.

---

## Combat System Comparison

### Classic Combat

**To-Hit System:**
- Roll 2d6 vs weapon's to-hit number
- Extensive modifier table:
  - Range: -1 per 4"
  - Target speed: -1 to -6
  - Firer speed: +1 if stationary
  - Target size: -1 to +3
  - Targeting computer: +1 to +3
  - Sustained fire: +1/+2
  - Maneuver penalty: -D value
  - Night/weather: -2 to -3

**Damage System:**
- Weapons have damage dice (1d, 2d, 3d, etc.)
- Damage penetrates armor → internal → components
- 6 armor locations (F, B, L, R, T, U)
- Damage location determined by attack angle
- Internal damage hits: weapons → power plant/crew/cargo → through to opposite side

**Fire Arcs:**
- Front weapons: forward 90°
- Side weapons: 90° per side
- Turret: 360°
- Dropped weapons: rear

### 6th Edition Combat

**Simplified Roll:**
- Attacker rolls colored dice from weapon card
- Count hits, add basic damage
- Defender rolls yellow dice equal to speed
- Shields negate hits
- Special damage types: tire damage, fire, explosion

**Damage System:**
- 4 armor sliders (F, B, L, R)
- When armor depleted, draw damage card
- Damage cards specify which internal component hit

---

## Key Differences Summary

| Aspect | Classic Compendium | 6th Edition |
|--------|-------------------|-------------|
| Design philosophy | Simulation | Beer & pretzels |
| Turn structure | 5 phases, speed-based | 1 phase, sequential |
| Vehicle construction | Detailed (chassis, engine, weight) | Card-based (build points) |
| Handling system | Numeric HS with recovery | Binary tokens |
| Maneuver difficulty | D0-D7+ | D1-D4 |
| Combat modifiers | 30+ factors | Dice colors |
| Armor locations | 6 | 4 |
| Weapon variety | 50+ types | Card deck |
| Cycles/Oversized | Full rules | Not included |
| Campaign play | Full economics | Not supported |

---

## Conclusion for Arena Combat Engine

**6th Edition is NOT suitable as a basis for your project because:**

1. **No 5-phase system** - Your multi-phase turn execution is a core feature
2. **No chassis/engine selection** - Your power plant validation (21 plants) would be wasted
3. **No weight/space calculations** - Your vehicle builder would have nothing to build
4. **Simplified handling** - Your HS tracking and control rolls have no equivalent
5. **No difficulty progression** - D1-D4 vs your D0-D7 maneuver range

**Continue with Classic Compendium rules.** Your current implementation aligns with the tactical depth that makes Car Wars distinctive.

---

## References

- Car Wars Compendium Second Edition (Steve Jackson Games, 1996)
- Car Wars 6th Edition Core Rules (Steve Jackson Games, 2022)
- [Car Wars Classic Rules PDF](https://www.sjgames.com/car-wars/games/classic/img/car-wars-classic-rules.pdf)
- [Car Wars 6th Edition Review](https://gamingtrend.com/reviews/car-wars-6th-edition-orange-purple-starter-set-expansions-review/)
- [BoardGameGeek - Car Wars 6th Edition](https://boardgamegeek.com/boardgame/291828/car-wars-sixth-edition)
- [Steve Jackson Games Forums](https://forums.sjgames.com/showthread.php?t=152997)
