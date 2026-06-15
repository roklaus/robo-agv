# AGV Line-Following Robot

> 🌐 Language: [Português](README.pt.md) | **English**

Academic embedded systems project — Automated Guided Vehicle (AGV)

## Description

Autonomous line-following robot capable of navigating a marked floor grid, executing a pre-determined path, and automatically avoiding obstacles. Implemented on Arduino using a finite state machine.

## Objective

Develop an AGV (Automated Guided Vehicle) capable of:
- Navigating along the lines of a 4×4 grid
- Executing a path defined by a sequence of decisions at each intersection
- Detecting and waiting for obstacle removal (anti-collision)
- Operating fully autonomously

## Grid

```
     E    F    G    H
1 ---+----+----+----+--- 5
     |    |    |    |
2 ---+----+----+----+--- 6
     |    |    |    |
3 ---+----+----+----+--- 7
     |    |    |    |
4 ---+----+----+----+--- 8
     A    B    C    D
```

## Hardware

| Component               | Description                                  |
|-------------------------|----------------------------------------------|
| Arduino Uno/Nano        | Main microcontroller                         |
| 2× IR line sensor       | Line reading (left and right)                |
| 1× IR counter sensor    | Intersection detection via interrupt         |
| 1× IR obstacle sensor   | Anti-collision via interrupt                 |
| 2× DC Motor             | Differential drive                           |
| H-bridge driver (L298N) | Motor direction and speed control            |

## Pin Mapping

| Pin | Function                 | Type   |
|-----|--------------------------|--------|
| 2   | Obstacle sensor          | INPUT  |
| 3   | Line counter sensor      | INPUT  |
| 5   | Right motor PWM          | OUTPUT |
| 6   | Left motor PWM           | OUTPUT |
| 7   | Left motor IN_1          | OUTPUT |
| 8   | Left motor IN_0          | OUTPUT |
| 9   | Right motor IN_1         | OUTPUT |
| 10  | Right motor IN_0         | OUTPUT |
| 11  | Right line sensor        | INPUT  |
| 12  | Left line sensor         | INPUT  |

## State Machine

```
FOLLOW
  │
  ├── intersection counted → decision = RIGHT   ──► TURN_R ──► back to FOLLOW
  ├── intersection counted → decision = LEFT    ──► TURN_L ──► back to FOLLOW
  ├── intersection counted → decision = END     ──► STOPPED_END (permanent)
  │
  └── obstacle detected ──► STOPPED ──► obstacle removed ──► back to previous state
```

### States

| State          | Description                                          |
|----------------|------------------------------------------------------|
| `SEGUIR`       | Follows the line using left and right sensors        |
| `CURVA_D`      | Executes a 90° right turn                            |
| `CURVA_E`      | Executes a 90° left turn                             |
| `PARADO`       | Stopped waiting for obstacle removal (resumable)     |
| `PARADO_FIM`   | Permanently stopped after completing the path        |

## Decision Sequence System

Each intersection detected by the counter sensor (`IR_CONTADOR_LINHA`) consumes one element from the sequence:

```cpp
int sequencia[] = {FRENTE, DIREITA, ESQUERDA, FRENTE, FRENTE};
```

| Value      | Action at intersection  |
|------------|-------------------------|
| `FRENTE`   | Go straight, no turn    |
| `DIREITA`  | Turn 90° right          |
| `ESQUERDA` | Turn 90° left           |
| `FIM`      | Stop permanently        |

### How to define the path

1. Identify the intersections the robot will cross, **in order**
2. For each one, define the action: `FRENTE`, `DIREITA`, `ESQUERDA` or `FIM`
3. Adjust the `sequencia[]` array and `totalPassos` in the code

**Example** — path `1 → E → 2 → B → 4`:
```
Intersection 1 (crossing at E): go straight  → FRENTE
Intersection 2 (crossing at 2): turn right   → DIREITA
Intersection 3 (crossing at B): turn right   → DIREITA
Intersection 4 (arrived at 4): stop          → FIM
```
```cpp
int sequencia[]   = {FRENTE, DIREITA, DIREITA, FIM};
const int totalPassos = 4;
```

## Anti-Collision

The obstacle sensor (pin 2) is connected via `CHANGE` interrupt:

- **Obstacle detected** (LOW signal): robot stops immediately, saves current state
- **Obstacle removed** (HIGH signal): robot resumes saved state automatically

```cpp
void obsISR()
{
  obstaculoAtivo = (digitalRead(IR_OBSTACULO) == LOW);
}
```

## Adjustable Parameters

```cpp
#define V_FRENTE_DIREITO   200   // right wheel speed on straight line
#define V_FRENTE_ESQUERDO  128   // left wheel speed on straight line

#define V_CURVA_D_RAPIDO   170   // fast wheel on right turn
#define V_CURVA_D_LENTO     10   // slow wheel on right turn

#define V_CURVA_E_RAPIDO   240   // fast wheel on left turn
#define V_CURVA_E_LENTO     10   // slow wheel on left turn

#define T_CURVA_D_MIN     1000   // minimum right turn duration (ms)
#define T_CURVA_E_MIN     1000   // minimum left turn duration (ms)

#define T_CURVA_D_MAX     7500   // right turn timeout (ms)
#define T_CURVA_E_MAX     7500   // left turn timeout (ms)
```

> Values vary between robots. Calibrate according to your hardware.

## Startup Delay

Setup waits **3 seconds** before activating intersection counting:

```cpp
delay(3000);
attachInterrupt(digitalPinToInterrupt(IR_CONTADOR_LINHA), contadorLinhaISR, RISING);
```

This allows positioning the robot at the starting point without triggering false counts.

## Repository Structure

```
robo-agv/
├── .github/
│   └── workflows/
│       └── language-switch.yml
├── robo_agv.ino   # Main Arduino code
├── README.md      # Active README
├── README.pt.md   # Portuguese version
└── README.en.md   # English version
```

## Evaluation Criteria Met

| Criterion                             | Points | Status |
|---------------------------------------|--------|--------|
| Line navigation                       | 1 pt   | ✅     |
| Execute pre-determined path           | 3 pts  | ✅     |
| Anti-collision system                 | 2 pts  | ✅     |
| Decision-making system                | 2 pts  | ✅     |
| Full autonomous execution             | 2 pts  | ✅     |
