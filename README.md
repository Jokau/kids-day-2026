# Kids Day 2026 — an RFID music box, protocol to be invented

A 4-hour workshop for ~10-year-olds: each group builds and flashes a real
Arduino machine that reads NFC cards with a phone and plays whatever message
is written on them — but **the musical language itself is the exercise**.
Nothing about which word means which note (or how long a note lasts) is
written anywhere: the kids invent the protocol, write it on cards AND in
code, and make the two sides agree. That's computer science: two talking
systems must share a convention.

## The hardware

Arduino Nano V4 3V (Joy-it ARD-NanoV4-3V-MC, ATmega328PB, 3.3 V logic) +
MFRC522 RFID reader + passive buzzer + LED, all on one breadboard:

```
RC522   : VCC → 3V3 | GND → GND | RST → D9 | SDA/SS → D10
          MOSI → D11 | MISO → D12 | SCK → D13      (IRQ: no wire)
Buzzer  : S → D8, middle → 3V3, − → GND
LED     : D2 → 220–330 Ω → LED → GND
Power   : USB-C (5 V)
```

Plan B (only if it must be): the same machine on a classic ESP32 devkit —
SS 5 / RST 21 / buzzer 25, `ledcWriteTone()` instead of `tone()`.

## The sketches

```
src/
├─ animateur/            ← adult side, never opened by the kids
│  ├─ 01_Test_Nano/      wiring check (buzzer + LED), flashed by a helper only
│  └─ 03_MusiqueCartes_Nano_COMPLETE/   the solved machine = demo machine
│                                       and answer key of the day
├─ enfants/              ← what the kids open and edit
│  └─ 02_MusiqueCartes_Nano/   the EMPTY machine: three exercise zones marked
│                              in the file (words + frequencies tables,
│                              duration convention, tempo)
└─ planB_ESP32/          the two machines from above, ESP32 twin versions
```

`02` (kids) and `03` (solved) are word-identical outside the protocole
zones — the solved version only fills in what the kids invent. The ESP32
versions are the same files word-for-word except the pins and
`ledcAttach()`/`ledcWriteTone()`.

## Building & uploading

Arduino IDE with the **MiniCore** package. The four Tools values, before
EVERY upload to the Nano:

- Board: MiniCore → **ATmega328**
- Clock: **External 8 MHz**
- Variant: **328PB** (default 328P fails with a signature-mismatch error)
- Port: the CH340 COM port of the board

Library needed: **MFRC522** (GithubCommunity). ESP32 plan B: core 3.x+
from Espressif, board « ESP32 Dev Module ».

## The cards

Any **MIFARE Classic 1K** card. Kids write plain text on cards with the
**NFC Tools** phone app (Write → Text) — one word, several words, or a
whole song, one phrase per line (a line break plays as a one-beat rest).

The machine reads sectors 1–3 and tries all three keys the card could
carry, because the key depends on WHO wrote the card last:

| who wrote the card          | sector key A     |
|-----------------------------|------------------|
| nobody (factory blank)      | `FFFFFFFFFFFF`   |
| the NFC Tools app           | `D3F7D3F7D3F7`   |
| other NDEF-standard writers | `A0A1A2A3A4A5`   |

MIFARE quirk (cost a full day of debugging): after a failed authentication
the card *closes* — every further command times out until it is woken and
re-selected again. `lireLaCarte()` handles this itself; wrong-key failures
show up as *timeouts*, not as clean auth errors.

## Where the rest lives

- `setup\` — the adult side: `SESSION_PLAN.md` (the full session design),
  `ARDUINO-INIT.md` (one-time IDE setup), the teacher's pre-flight ear check.
- `kid resources\` — everything the kids receive or that gets printed: the
  workshop booklet (`manuel_enfant`, HTML source + generated PDF) and the
  wiring cards / room rules / kit checklist (`fiches_atelier`).
- `resources\` — read-only hardware datasheets and manuals.
