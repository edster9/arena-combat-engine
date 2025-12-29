# Car Wars Maneuvering System

This document covers all vehicle maneuvers from the Car Wars Compendium 2nd Edition, including rules, difficulty ratings, speed requirements, and implementation notes.

---

## Table of Contents

1. [Overview](#overview)
2. [Handling System](#handling-system)
3. [Basic Maneuvers](#basic-maneuvers)
4. [Advanced Maneuvers](#advanced-maneuvers)
5. [Special Maneuvers](#special-maneuvers)
6. [Control & Crash Tables](#control--crash-tables)
7. [Implementation Status](#implementation-status)

---

## Overview

### Core Concepts

| Term | Meaning |
|------|---------|
| **D Rating** | Difficulty Class - how much handling status is reduced |
| **HC** | Handling Class - base maneuverability from suspension |
| **Handling Status** | Current control level (starts at HC, reduced by maneuvers) |
| **Control Roll** | 1d6 roll to maintain control when handling status drops |

### Movement Basics

- **1 turn = 1 second = 5 phases**
- **10 mph = 1 inch/turn**
- **1 inch = 15 feet** (1:180 scale)
- **One maneuver per phase max**
- **Half-inch moves cannot maneuver** (exception: Pivot at 5 mph)

### Maneuver Timing

1. Announce speed change (accel/decel) at start of phase
2. Make control roll if handling status dropped
3. Execute movement with maneuver
4. At end of turn: recover handling status (+HC, min +1)

---

## Handling System

### Handling Class (HC)

Base HC comes from suspension type:

| Suspension | Car HC | Van HC | Subcompact HC |
|------------|--------|--------|---------------|
| Light | 1 | 0 | 2 |
| Improved | 2 | 1 | 3 |
| Heavy | 3 | 2 | 4 |
| Off-Road | 2 | 1 | 3 |

**Modifiers:**
- Driver reflex roll: 5 = +1 HC, 6+ = +2 HC
- Driver skill bonus added to reflex roll
- Spoiler/Airdam: +1 HC at 60+ mph
- HD Shocks: +1 HC
- Radial tires (all wheels): +1 HC
- Racing slicks (all wheels): +2 HC

### Handling Status

- **Start**: Equals modified Handling Class
- **Reduction**: Decreased by maneuver Difficulty or hazard
- **Minimum**: -6 (still must roll on Control Table)
- **Recovery**: At end of each turn: +HC (min +1), cannot exceed starting HC

### When to Roll

After each maneuver or hazard, if handling status dropped:
1. Cross-reference speed and new handling status on Control Table
2. If a number is shown, roll 1d6 - must equal or exceed that number
3. If "XX" is shown, automatic loss of control
4. If "safe", no roll needed

---

## Basic Maneuvers

### Drift (D1)

**Description:** Move 1" forward and up to 1/4" sideways while maintaining the same heading.

| Attribute | Value |
|-----------|-------|
| Difficulty | D1 |
| Speed Requirement | Any |
| Lateral Movement | Up to 1/4" |
| Heading Change | None |

**Use Case:** Lane changes, minor position adjustments.

```
     ___          ___
    |   |   →    |   |
    |___|        |___|
      ↓            ↓
    Start    1/4" right, same heading
```

---

### Steep Drift (D3)

**Description:** Move 1" forward and 1/4" to 1/2" sideways while maintaining the same heading.

| Attribute | Value |
|-----------|-------|
| Difficulty | D3 |
| Speed Requirement | Any |
| Lateral Movement | 1/4" to 1/2" |
| Heading Change | None |

**Use Case:** Aggressive lane changes, dodging.

---

### Bend (D1-D6)

**Description:** Vehicle moves 1" forward, then angles to one side, keeping one rear corner fixed in place.

| Bend Angle | Difficulty |
|------------|------------|
| Up to 15° | D1 |
| 16° to 30° | D2 |
| 31° to 45° | D3 |
| 46° to 60° | D4 |
| 61° to 75° | D5 |
| 76° to 90° | D6 |

**Use Case:** Cornering, turns.

```
           ___
          /   \     ← 45° bend (D3)
         |     |
    ___  |_____|
   |   |    ↑
   |___|  pivot point (rear corner)
```

---

### Swerve (Bend D + 1)

**Description:** A 1/4" drift followed by a bend in the **opposite direction**, all in the same phase.

| Attribute | Value |
|-----------|-------|
| Difficulty | Bend difficulty + 1 |
| Speed Requirement | Any |
| Components | 1/4" drift + opposite bend |

**Example:** Drift right 1/4", then bend 30° left = D2 + 1 = D3

**Use Case:** Quick direction change, dodging obstacles.

```
    ___
   |   | ← drift right
   |___|
     ↓
    ___
   /   \  ← then bend left
  |_____|
```

---

## Advanced Maneuvers

### Controlled Skid (D+1 to D+4)

**Description:** Perform a bend or swerve, then deliberately skid in the **original direction** of travel.

**Execution:**
1. Perform bend or swerve
2. Declare skid distance (1/4", 1/2", 3/4", or 1")
3. Make control roll for combined difficulty
4. On next move: skid the declared distance, then move remainder straight

| Skid Distance | Added D | Weapon Penalty | Decel | Tire Damage |
|---------------|---------|----------------|-------|-------------|
| 1/4" | +D1 | -1 to aimed | 0 mph | 0 |
| 1/2" | +D2 | -3 to aimed | 5 mph | 0 |
| 3/4" | +D3 | -6 to aimed | 5 mph | 1 per tire |
| 1" | +D4 | No aimed fire | 10 mph | 2 per tire |

**Example:** 45° bend (D3) + 1/2" skid = D3 + D2 = D5 total

**Use Case:** Sharp turns while maintaining original trajectory, powerslides.

---

### Deceleration (D varies)

**Description:** Rapid braking counts as a maneuver with difficulty based on speed lost.

| Speed Change | Standard Brakes | HD Brakes | Tire Damage |
|--------------|-----------------|-----------|-------------|
| -5 mph | - | - | 0 |
| -10 mph | D1 | D1 | 0 |
| -15 mph | D2 | D2 | 0 |
| -20 mph | D3 | D3 | 0 |
| -25 mph | D5 | D5 | 0 |
| -30 mph | D7 | D7 | 0 |
| -35 mph | D9* | D7* | 2 per tire |
| -40 mph | D11** | D9** | 1d per tire |
| -45 mph | - | D11*** | 1d+3 per tire |
| -50 mph | - | - | - |

**Notes:**
- Deceleration does NOT count as your one maneuver per phase
- Deceleration IS treated as a maneuver for Crash Table purposes
- ABS (Anti-lock Brakes) negates tire damage from braking
- Max decel without HD brakes: 40 mph
- Max decel with HD brakes: 45 mph

---

## Special Maneuvers

### Pivot (D0) - 5 mph ONLY

**Description:** At exactly 5 mph, move 1/4" forward, then pivot any amount around one rear corner.

| Attribute | Value |
|-----------|-------|
| Difficulty | D0 |
| Speed Requirement | **Exactly 5 mph** |
| Rotation | Any amount, any direction |

**Restrictions:**
- Only available at 5 mph
- Alternative to the normal 1/2" half-move

**Use Case:** Tight parking, precise positioning, sharp turns at low speed.

```
    ___         ___
   |   |       /   \
   |___|  →   |     |  ← any rotation
     ●        |_____|
   pivot        ●
   point      same point
```

---

### T-Stop (D1 per 10 mph)

**Description:** Emergency stop by rotating 90° and skidding sideways until halted.

| Attribute | Value |
|-----------|-------|
| Difficulty | D1 per 10 mph of deceleration |
| Speed Requirement | Any (but only if can do Bootlegger) |
| Deceleration | 20 mph per inch of movement |
| Tire Damage | 1 per tire per 20 mph lost |

**Restrictions:**
- Cannot fire aimed weapons during T-Stop
- Only vehicles that can perform Bootlegger can T-Stop
- Cycles and oversized vehicles cannot perform

**Example:** At 60 mph, T-Stop is D6. Decelerates 20 mph per inch moved (3 inches to stop), tires take 3 damage each.

**Use Case:** Emergency stops, dramatic halts.

```
    ___
   |   |  60 mph forward
   |___|
     ↓
   _____
  |     |  rotate 90°, skid sideways
  |_____|
     ↓
  [STOP]  after 3 inches of sideways movement
```

---

### Bootlegger Reverse (D7) - 20-35 mph ONLY

**Description:** The classic J-turn. A controlled skid to reverse direction, ending at 0 mph facing the opposite way.

| Attribute | Value |
|-----------|-------|
| Difficulty | **D7** |
| Speed Requirement | **20-35 mph ONLY** |
| Tire Damage | 1 per tire (immediate, when starting) |
| Ending Speed | 0 mph |
| Ending Direction | 180° reversed |

**Precise Execution (from Reference Booklet):**

**Phase 1 - Start the Bootlegger:**
1. Move forward **3/4"**
2. Rotate **90°** in the direction you wish to turn
3. All tires take 1 point damage immediately

**Phase 2 - Complete the Bootlegger (next full inch of movement):**
1. Move **1/2"** forward (in new facing direction)
2. Move **3/4"** in your **original movement direction** (sideways)
3. Rotate another **90°** (now facing opposite original direction)
4. Set speed to **0 mph**

```
    Phase 1:                    Phase 2:
         ___
        |   | → 3/4" forward        ___
        |___|                      |   | 0 mph
          ↓                        |___|
         ___                         ↑
        /   | 90° rotation       1/2" fwd + 3/4" sideways
        |___|                    + 90° more rotation
```

**Restrictions:**
- Cannot fire aimed weapons until fully stopped
- Automatic weapons still fire
- Cycles and oversized vehicles CANNOT attempt
- No other maneuvers can combine with bootlegger
- Cannot slow to 35 mph and then attempt in same turn

**On Failure:**
- Move to first position (3/4" forward + 90° rotation)
- Apply crash table results
- Movement vector is **sideways** (like T-Stop)
- Speed loss only as dictated by crash result

**Use Case:** Escaping pursuit, reversing direction quickly. On a two-lane road, ends in correct lane for new direction.

---

### Movement in Reverse

**Description:** Any vehicle except cycles can move backwards.

| Attribute | Value |
|-----------|-------|
| Max Speed | 1/5 of top speed (rounded down) |
| Direction Change | Must stop for 1 full turn to switch |
| Maneuver Modifier | +D1 to all maneuvers |

**Example:** Top speed 100 mph → max reverse 20 mph

---

## Control & Crash Tables

### Control Table (Simplified)

Cross-reference speed and handling status:

| Speed | HC 2+ | HC 1 | HC 0 | HC -1 | HC -2 | HC -3 | HC -4 | HC -5 | HC -6 |
|-------|-------|------|------|-------|-------|-------|-------|-------|-------|
| 5-20 | safe | safe | safe | safe | 2+ | 3+ | 4+ | 5+ | XX |
| 25-40 | safe | safe | safe | 2+ | 3+ | 4+ | 5+ | XX | XX |
| 45-60 | safe | safe | 2+ | 3+ | 4+ | 5+ | XX | XX | XX |
| 65-80 | safe | 2+ | 3+ | 4+ | 5+ | XX | XX | XX | XX |
| 85-100 | 2+ | 3+ | 4+ | 5+ | XX | XX | XX | XX | XX |

- **safe** = no roll needed
- **2+** to **5+** = must roll that or higher on 1d6
- **XX** = automatic loss of control → Crash Table

### Crash Table Modifiers

| Source | Modifier |
|--------|----------|
| Speed 5-10 mph | -3 |
| Speed 15-20 mph | -2 |
| Speed 25-30 mph | -1 |
| Speed 35-40 mph | +0 |
| Speed 45-60 mph | +1 |
| Speed 65-80 mph | +2 |
| Speed 85-100 mph | +3 |
| Driver skill | -skill |
| Maneuver D > 3 | +(D - 3) |

### Crash Table 1 - Maneuver Crashes (Roll 2d6 + modifiers)

| Roll | Result | Weapon Penalty | Effect |
|------|--------|----------------|--------|
| <3 | Trivial Skid | -3 aimed | Skid 1/4", no speed loss |
| 3-4 | Minor Skid | -6 aimed | Skid 1/2", -5 mph |
| 5-6 | Moderate Skid | -6 aimed | Skid 3/4", -10 mph, 1 tire damage, trivial skid next inch |
| 7-8 | Severe Skid | No aimed | Skid 1", -20 mph, 2 tire damage, minor skid next inch |
| 9-10 | Spinout | No aimed | Rotate 90°, 1d tire damage, move 1"/phase along vector, -20 mph every 5 phases |
| 11-12 | Roll | No aimed | Vehicle rolls sideways, 1d damage per side, -20 mph every 5 phases |
| 13-14 | Burning Roll | No aimed | As Roll, but fire on 4-6 (1d6) |
| 15+ | Vault | No aimed | Tires on leading edge take 3d, fly 1d6", rotate 2 sides/inch, land and continue as Roll |

**Crash Result Details:**

- **Skids**: Happen on next phase. Complete your maneuver unless T-Stop or Bootlegger.
- **Spinout**: Replaces maneuver. Spin in fishtail direction (or random). After first rotation, make control roll at HS -6 every 5 phases. If you regain control facing forward/backward, continue normally; if sideways, either T-Stop or turn into skid.
- **Roll**: Replaces maneuver. Vehicle turns sideways, rolls 1"/phase. Each side hit takes 1d. When rolling onto underbody, each tire takes 1d until gone. Cycles are not drivable after a roll.
- **Vault**: Replaces maneuver. If caused by D3+ bend, vehicle flips end-over-end. When landing, all occupants take 1 point (ignoring armor), vehicle takes collision damage at current speed.

### Crash Table 2 - Hazard Crashes (Roll 2d6 + modifiers)

| Roll | Result | Weapon Penalty | Effect |
|------|--------|----------------|--------|
| 1-4 | Minor Fishtail | -3 aimed | Fishtail 1/4" |
| 5-8 | Major Fishtail | -6 aimed | Fishtail 1/2" |
| 9-10 | Minor Fishtail + Table 1 | No aimed | Fishtail 1/4", then roll on Crash Table 1 |
| 11-14 | Major Fishtail + Table 1 | No aimed | Fishtail 1/2", then roll on Crash Table 1 |
| 15+ | Extreme Fishtail + Table 1 | No aimed | Fishtail 3/4", then roll on Crash Table 1 |

**Fishtail Mechanics:**

To perform a fishtail:
1. Hold down the **opposite front corner** from the fishtail direction
2. Rotate the rear end until the back corner has slid the specified distance
3. Fishtail direction is always **random** (1-3 left, 4-6 right)

```
     Before              After (Minor Fishtail Right)
      ___                    ___
     |   |                  /   |
     |___|                 |____|
       ↓                     ↓
    Front corner          Front corner stays fixed,
    stays fixed           rear swings 1/4" right
```

**Important Rules:**
- Apply fishtails **immediately**, before rolling again
- If a fishtail causes a new collision, finish crash results first, then resolve collision
- If already in spinout/roll/vault, don't apply additional fishtails

### Hazards

External events that reduce handling status:

| Hazard | Difficulty | Additional Effect |
|--------|------------|-------------------|
| 1-5 damage taken | D1 | - |
| 6-9 damage taken | D2 | - |
| 10-19 damage taken | D3 | Drop Debris |
| 20+ damage taken | D3 | Drop Obstacle |
| Hitting Debris | D1 | D6-3 damage to all tires |
| Hitting Obstacle | D3 | D6-3 damage to all tires |
| Hitting Pedestrian or Curb | D3 | Ram pedestrian |
| Driver injured or killed | D2 | - |
| Loss of first Plasticore rubber | - | -1 Max HC |
| Loss of any tire | - | Drop Obstacle |
| Loss of last Radial on location | - | -1 Max HC |
| Loss of last Slick on location | - | -2 Max HC |
| Loss of first tire in group (not only) | D2 | - |
| Loss of more tires in group (not last) | D3 | - |
| Loss of last tire on location | D6 | Reduce HS to -6 |
| Loss of all wheels on one location | D6 | -3 Max HC, HS to -6 |
| Loss of all wheels on two locations | Crash Table 1 | Decel 30 every 5 phases, no maneuvers, HS permanently -6 |

### Road Conditions

| Condition | Added Difficulty |
|-----------|-----------------|
| Off-road | +D1 to maneuvers |
| Light rain | +D1 to all |
| Heavy rain | +D2 to all |
| Gravel | +D1 to all |
| Oil | +D2 to all |
| Light snow | +D2 to all |
| Heavy snow | +D3 to all |
| Ice | +D4 to all |

---

## Implementation Status

### Phase 1: Foundation
- [ ] Handling system (HC tracking, status, recovery)
- [ ] Control roll system (1d6 vs table)
- [ ] Crash table resolution

### Phase 2: Basic Maneuvers
- [ ] Pivot (D0, 5mph only) - simplest
- [ ] Drift (D1) - lateral movement
- [ ] Steep Drift (D3)
- [ ] Bend (D1-D6) - core turning

### Phase 3: Compound Maneuvers
- [ ] Swerve (D+1) - drift + opposite bend
- [ ] Controlled Skid (D+1 to D+4)
- [ ] Deceleration as maneuver

### Phase 4: Special Maneuvers
- [ ] T-Stop (D1/10mph)
- [ ] Bootlegger Reverse (D7, 20-35mph)
- [ ] Reverse movement (+D1 modifier)

### Phase 5: Environmental
- [ ] Road condition modifiers
- [ ] Hazard system
- [ ] Crash consequences

---

## Physics Implementation Notes

### Current State

The physics engine uses "matchbox car" model:
- Direct force applied to center of mass
- Wheels are cosmetic (steering via constraint)
- No weight transfer from acceleration

### Maneuver Integration

For each maneuver, we need to:
1. **Detect eligibility** - speed requirements, vehicle type
2. **Apply physics** - lateral forces, rotation torques
3. **Track D rating** - accumulate difficulty
4. **Trigger control roll** - when handling status drops
5. **Apply consequences** - skids, crashes, tire damage

### Key Physics Functions Needed

```c
// Check if maneuver can be attempted
bool maneuver_can_attempt(PhysicsVehicle* v, ManeuverType type);

// Execute maneuver (apply forces/torques)
void maneuver_execute(PhysicsVehicle* v, ManeuverType type, float param);

// Get resulting difficulty
int maneuver_get_difficulty(ManeuverType type, float param);

// Make control roll
bool handling_control_roll(PhysicsVehicle* v, int speed_mph, int handling_status);

// Apply crash result
void crash_apply_result(PhysicsVehicle* v, int crash_roll);
```

---

---

## Skid Execution

When performing any skid (from Controlled Skid or Crash Table):

1. Line up the turning key with the vehicle's **previous movement vector**
2. Slide the counter along the turning key the skid distance
3. Reduce speed and take tire damage (if any)
4. If movement remaining, take it **after** the skid (straight ahead)

```
    Before Skid              During Skid              After Skid
        ___                      ___                     ___
       |   | → moving           |   | → skid along      |   | → continue
       |___|   this way         |___|   original vector |___|   forward
```

---

## References

- Car Wars Compendium 2nd Edition, Steve Jackson Games
- Car Wars Reference Booklet 3x (cwspark.org)
- [books/Car_Wars_Compendium.txt](../books/Car_Wars_Compendium.txt)
- [books/cwc_reference_booklet_3x.pdf](../books/cwc_reference_booklet_3x.pdf)
- [data/carwars_db.json](../data/carwars_db.json) - Movement and maneuvers section
