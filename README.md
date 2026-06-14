# Automatic CPR Portable

Firmware for **Automatic CPR Portable Dilengkapi dengan Sensor Denyut Jantung** — Arduino Uno prototype with MAX30102 heart-rate monitoring, I2C LCD, TB6600 stepper belt compression, push buttons, and active buzzer.

## Project structure

```
test-max30102/
├── platformio.ini          # Board, framework, library deps
├── src/
│   ├── main.cpp            # Setup / loop entry point
│   ├── cpr_engine.cpp      # State machine (idle → pulse check → CPR)
│   ├── pulse_sensor.cpp    # MAX30102 init and BPM averaging
│   ├── stepper_motor.cpp   # TB6600 timed step pulses
│   ├── display.cpp         # I2C LCD messages
│   ├── buttons.cpp         # Debounced button polling
│   └── buzzer.cpp          # Ventilation / stop beeps
└── include/
    ├── config.h            # Pins, timing, CPR constants (edit here)
    ├── cpr_engine.h
    ├── pulse_sensor.h
    ├── stepper_motor.h
    ├── display.h
    ├── buttons.h
    └── buzzer.h
```

| Module | Responsibility |
|--------|----------------|
| `cpr_engine` | System state machine and compression batch orchestration |
| `pulse_sensor` | MAX30102 setup, beat detection, rolling BPM average |
| `stepper_motor` | TB6600 enable/step/dir control; ENA on only during compression strokes |
| `display` | LCD screens for each state |
| `buttons` | Debounced reads for Start / Gemuk / Stop / Kurus |
| `buzzer` | Active buzzer on/off and multi-beep patterns |

## Hardware

| Component | Connection |
|-----------|------------|
| Arduino Uno | Main controller |
| MAX30102 | 5V, GND, SDA → A4, SCL → A5 |
| LCD 16×2 I2C | Same I2C bus, address `0x27` (try `0x3F` if blank) |
| Active buzzer | D13 → buzzer+, GND → buzzer− |
| TB6600 PUL | PUL+ → D9 (`STEP_PIN`), PUL− → GND (with DIR−) |
| TB6600 DIR | DIR+ → D8 (`DIR_PIN`), DIR− → GND (with PUL−) |
| TB6600 ENA | ENA+ → D10 (`ENABLE_PIN`), ENA− → GND |
| TB6600 common | PUL−, DIR−, and ENA− to Arduino GND; driver GND tied to Arduino GND |
| Stepper NEMA 23 | A+/A−, B+/B− to driver |
| Motor PSU | 9–42 V DC to driver VCC/GND (**not** Arduino 5V) |

### TB6600 enable (ENA)

The driver is **disabled at boot and idle** so the motor coils are not energized between compressions. `stepMotorTimed()` turns ENA **on** at the start of each stroke and **off** when the stroke finishes (or when **Stop** is pressed mid-stroke).

| Situation | Expected |
|-----------|----------|
| Idle (Arduino running, motor PSU on) | Shaft spins easily by hand; little or no chopper hum |
| During compression stroke | Shaft resists; driver hums while stepping |
| Motor PSU only (Arduino off) | Driver may stay enabled — **not** a valid ENA test |

If the shaft stays stiff at idle, toggle `TB6600_ENABLE_5V` in `config.h`. If still stiff, check ENA+ on D10, ENA− on GND, and common GND to the driver.

If your board uses PUL+/DIR+ to 5V instead (PUL−/DIR− on GPIO), set `TB6600_COMMON_5V` to `true` and re-check ENA polarity.

### Buttons (INPUT_PULLUP — pressed = LOW)

| Pin | Function |
|-----|----------|
| D2 | **Start** — begin pulse check |
| D4 | **Gemuk** — 30 compressions per cycle |
| D7 | **Stop** — abort CPR |
| D12 | **Kurus** — 15 compressions per cycle |

One side of each button → Arduino pin, other side → GND.

## System flow

```
Power on → Idle (show BPM)
    ↓ [Start]
Pulse check (2.5 s)
    ├─ Pulse detected → "Pasien Masih Hidup" → Idle
    └─ No pulse       → "Pasien Henti Jantung" → pick Gemuk or Kurus
                              ↓
                    Compression loop (until Stop)
                    ├─ Gemuk: 30 kompresi
                    └─ Kurus: 15 kompresi
                              ↓
                    Buzzer 2× (ventilasi cue)
                              ↓
                    Repeat or Stop → Idle
```

### LCD messages

| State | Line 1 | Line 2 |
|-------|--------|--------|
| Idle | `ALAT CPR OTOMAT` | `BPM: xx` |
| Checking | `Cek denyut...` | `BPM: xx` |
| Alive | `Pasien Masih Hid` | `BPM: xx` |
| Arrest | `Pasien Henti Jnt` | `Gemuk / Kurus` |
| Gemuk CPR | `Mode Gemuk` | `Komp: n/30` |
| Kurus CPR | `Mode Kurus` | `Komp: n/15` |
| Stopped | `CPR dihentikan` | (blank) |

## Tunable parameters

Edit constants in [`include/config.h`](include/config.h):

| Constant | Default | Purpose |
|----------|---------|---------|
| `GEMUK_COMPRESSIONS` | 30 | Compressions per cycle (gemuk / larger torso) |
| `KURUS_COMPRESSIONS` | 15 | Compressions per cycle (kurus / smaller torso) |
| `COMPRESSIONS_PER_MIN` | 110 | Target rate (AHA: 100–120/min) |
| `STEPS_PER_STROKE` | 200 | Motor steps per belt direction — tune to belt travel |
| `STEPS_PER_REV` | 200 | Must match TB6600 DIP switch microstep setting |
| `FINGER_IR_MIN` | 70000 | IR threshold for finger / pulse detection |
| `PULSE_BPM_MIN` | 20 | Minimum BPM to count as detectable pulse |
| `ENABLE_PIN` | `10` | TB6600 ENA+ (Arduino GPIO) |
| `TB6600_COMMON_5V` | `false` | Set `true` if PUL+/DIR+ go to 5V and PUL−/DIR− go to GPIO |
| `TB6600_ENABLE_5V` | `false` | ENA opto polarity — toggle if idle shaft stays stiff (driver still on) |

## Build and upload

Requires [PlatformIO](https://platformio.org/).

```bash
cd test-max30102
pio run              # build
pio run -t upload    # flash to Arduino
pio device monitor   # serial log @ 115200 baud
```

Or from VS Code / Cursor: **PlatformIO → Build**, then **Upload**.

## Serial monitor

At 115200 baud the firmware prints state, IR value, and BPM each UI tick:

```
state=0 IR=125000 BPM=72
```

State values: `0` Idle, `1` PulseCheck, `2` AwaitingMode, `3` RunningGemuk, `4` RunningKurus.

## Test procedure

1. **Idle** — LCD shows title and BPM when finger is on MAX30102.
2. **Alive path** — Finger on sensor → press **Start** → `Pasien Masih Hidup`, motor does not run.
3. **Arrest path** — No finger → **Start** → `Pasien Henti Jantung` → press **Gemuk** or **Kurus**.
4. **Compression** — Motor runs tighten/release strokes; LCD counts `Komp: n/total`.
5. **Ventilation cue** — After each batch, buzzer beeps twice.
6. **Stop** — Press **Stop** anytime → motor halts, short buzzer chirp, returns to idle after 2 s.

## Dependencies

From [`platformio.ini`](platformio.ini):

- `sparkfun/SparkFun MAX3010x Pulse and Proximity Sensor Library`
- `marcoschwartz/LiquidCrystal_I2C`

## Notes

- CPR is **button-driven**, not auto-triggered by low BPM.
- Buzzer cues ventilation only; there is no bag-valve hardware.
- Belt depth (5–6 cm target) is set mechanically via `STEPS_PER_STROKE` and mechanical linkage, not in software.
- If motor does not move, verify TB6600 opto wiring (GND common vs 5V common) and set `TB6600_COMMON_5V` accordingly.
- Connect ENA+ to D10 and ENA− to GND for software enable control. Leaving ENA unconnected keeps the driver always enabled.
- Motor PSU alone can produce chopper hum even when idle; verify ENA with **both** PSUs on and Arduino at idle (shaft should turn freely).

`.pio/` build cache and local VS Code generated files are gitignored. Run `pio run` after clone to restore dependencies.
