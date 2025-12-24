# Game Design: Arena Duel Demo

## Overview

The Arena Duel is the core gameplay loop of Car Wars. Two or more armed vehicles enter an enclosed arena and fight until one remains operational. This document defines the mechanics for our initial demo.

---

## Scope: Demo vs Full Game

| Feature | Demo (Phase 1) | Full Game (Future) |
|---------|----------------|-------------------|
| Vehicles | 2 (1v1) | Many, team battles |
| Weapons | 1-2 types | Full catalog |
| Arena | 1 simple arena | Multiple arenas |
| Damage | Simplified | Full component damage |
| AI | Basic or hotseat | Full tactical AI |
| Vehicle builder | None (preset cars) | Full customization |

---

## Turn Structure

### DECISION: Simultaneous Resolution

This matches the tabletop experience - everyone commits their moves, then the turn plays out.

```
┌─────────────────────────────────────────────────────┐
│  TURN SEQUENCE                                      │
├─────────────────────────────────────────────────────┤
│  1. PLANNING PHASE (Timed)                          │
│     - All players (human + NPC) choose:             │
│       • Speed change (accel/decel/maintain)         │
│       • Maneuvers for the turn                      │
│       • Weapon targets                              │
│     - Optional timer for decision (15-30 sec?)      │
│     - NPC "thinks" during same window               │
│                                                     │
│  2. EXECUTION PHASE                                 │
│     - Movements resolved simultaneously             │
│     - Higher speed moves first in each sub-phase    │
│     - Collisions resolved as they occur             │
│     - Camera follows action cinematically           │
│                                                     │
│  3. COMBAT PHASE                                    │
│     - All weapons fire (simultaneous)               │
│     - Dice rolls shown briefly (damage resolution)  │
│     - Damage applied                                │
│     - Status effects resolved (fire, etc.)          │
│                                                     │
│  4. END PHASE                                       │
│     - Check victory conditions                      │
│     - Advance turn counter                          │
│     - Return to planning                            │
└─────────────────────────────────────────────────────┘
```

**Why this approach:**
- Authentic to tabletop: everyone commits, then watch it unfold
- Tension: "will they go left or right?" is the game
- Network-ready: same structure works for online play
- NPC fits naturally: AI plans during same window as human

---

## Movement System

### Speed

Vehicles have a current speed measured in "inches" per turn (borrowing tabletop terminology).

| Speed | Movement | Maneuverability |
|-------|----------|-----------------|
| 0-10 mph | Very slow | Can turn freely |
| 15-30 mph | Slow | Easy handling |
| 35-50 mph | Medium | Standard handling |
| 55-70 mph | Fast | Difficult turns |
| 75+ mph | Very fast | Dangerous maneuvers |

### Acceleration/Deceleration

Per turn, a vehicle can:
- **Accelerate:** +5 to +15 mph (depends on engine)
- **Decelerate:** -5 to -10 mph (braking)
- **Hard brake:** More, but risks control loss
- **Maintain:** No change

### Maneuvers

Borrowing from Car Wars:

| Maneuver | Difficulty | Description |
|----------|------------|-------------|
| Straight | Easy | No turn |
| Slight turn | Easy | ~15 degrees |
| Standard turn | Moderate | ~45 degrees |
| Hard turn | Difficult | ~90 degrees |
| Bootlegger reverse | Very hard | 180 spin |

Higher speed + harder maneuver = risk of losing control.

### Control Rolls

When a vehicle attempts a difficult maneuver:

```
Roll = Base Handling + Driver Skill - Speed Penalty - Maneuver Difficulty

If Roll < Threshold:
  - Minor fail: Skid (lose some control of direction)
  - Major fail: Spin (lose a turn, face random direction)
  - Critical fail: Roll (vehicle flips, major damage)
```

---

## Combat System

### Weapon Types (Demo)

| Weapon | Range | Damage | Notes |
|--------|-------|--------|-------|
| Machine Gun | Medium | Low per hit, many hits | Forward fixed |
| Rocket Launcher | Long | High, single shot | Limited ammo |

### Firing Arcs

Weapons are mounted in specific positions:

```
        FRONT
          ▲
     ┌────┴────┐
     │    F    │  F = Front arc (60°)
   L │         │ R  L/R = Side arcs (60° each)
     │    B    │  B = Back arc (60°)
     └────┬────┘
          ▼
        BACK
```

Turret-mounted weapons (future) can fire in any direction.

### To-Hit Calculation

```
Base chance: 50%
Modifiers:
  + Shooter skill
  + Target size
  - Range penalty
  - Target speed
  - Shooter speed
  - Maneuver penalty
```

### Damage Application (Simplified for Demo)

Instead of full component damage:

```
Vehicle has:
  - Armor (front, back, left, right) - absorbs damage
  - Hull (structure) - destroyed when depleted

Damage flow:
  1. Hit location determined by facing
  2. Damage reduces armor on that side
  3. If armor depleted, damage goes to hull
  4. Hull at 0 = vehicle destroyed
```

Full game would add: tires, weapons, engine, crew, cargo, etc.

---

## Victory Conditions

### Demo Win States

| Condition | Result |
|-----------|--------|
| Enemy hull at 0 | You win |
| Your hull at 0 | You lose |
| Both at 0 same turn | Draw |
| Timer expires | Higher hull % wins |

### Future Win States

- Last team standing
- Reach X kills
- Survive X turns
- Destroy objective
- Escape the arena

---

## Arena Design (Demo)

### Simple Arena

```
┌─────────────────────────────────────────┐
│▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓│
│▓                                       ▓│
│▓   ┌───┐                     ┌───┐     ▓│
│▓   │ O │                     │ O │     ▓│
│▓   └───┘                     └───┘     ▓│
│▓                                       ▓│
│▓           P1 START ►                  ▓│
│▓                                       ▓│
│▓                  ┌───┐                ▓│
│▓                  │ O │                ▓│
│▓                  └───┘                ▓│
│▓                                       ▓│
│▓                ◄ P2 START             ▓│
│▓                                       ▓│
│▓   ┌───┐                     ┌───┐     ▓│
│▓   │ O │                     │ O │     ▓│
│▓   └───┘                     └───┘     ▓│
│▓                                       ▓│
│▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓│
└─────────────────────────────────────────┘

▓ = Arena wall (solid, no ramming through)
O = Obstacle (concrete block, can take cover behind)
P1/P2 = Starting positions (facing each other)
```

### Arena Properties

- **Size:** ~40x30 grid units
- **Walls:** Indestructible, damage on collision
- **Obstacles:**
  - Block line of sight
  - Block movement
  - Provide cover
  - Take damage (eventually destructible?)
- **Floor:** Flat for demo (ramps/jumps future)

---

## Vehicle Definitions (Demo)

### Vehicle 1: "Striker"

Fast, light, machine gun focus

```
Chassis:    Compact
Top Speed:  95 mph
Accel:      15 mph/turn
Handling:   +2
Armor:      F:10 / B:5 / L:5 / R:5
Hull:       20
Weapons:    2x MG (front)
```

### Vehicle 2: "Bruiser"

Slow, heavy, rocket launcher

```
Chassis:    Sedan
Top Speed:  70 mph
Accel:      10 mph/turn
Handling:   0
Armor:      F:15 / B:10 / L:10 / R:10
Hull:       35
Weapons:    Rocket Launcher (front), 1x MG (rear)
```

---

## UI Requirements (Demo)

### Planning Phase UI

```
┌─────────────────────────────────────────┐
│  YOUR TURN - PLANNING PHASE             │
├─────────────────────────────────────────┤
│                                         │
│  SPEED: [◄ BRAKE] [45 mph] [ACCEL ►]   │
│                                         │
│  MANEUVERS:                             │
│  [STRAIGHT] [SLIGHT L] [SLIGHT R]       │
│  [TURN L] [TURN R]                      │
│  [HARD L] [HARD R]                      │
│                                         │
│  WEAPONS:                               │
│  [MG: Click arena to target]            │
│  [ROCKET: Click arena to target]        │
│                                         │
│  [LOCK IN ORDERS]                       │
└─────────────────────────────────────────┘
```

### Combat Display

- Current speed shown on vehicle
- Armor remaining (visual indicator)
- Movement path preview
- Firing arcs visible when targeting
- Hit/miss feedback
- Damage numbers

---

## Open Questions

- [ ] Grid size: 1 inch = ? pixels/units
- [ ] Precise control roll formula
- [ ] Collision damage calculation
- [ ] Does ramming work in demo?
- [ ] How do we show simultaneous movement? (Replay? Animation?)
- [ ] Hotseat: Same screen or hidden orders?
