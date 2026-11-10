# Arduino IDE — installation & first flash (Nano V4 3V)

Operator's sheet for the teacher: everything to install once, and the exact
Tools settings for every upload. The full workshop logic lives in
`SESSION_PLAN.md`; this file is only the "make the laptop able to program" part.

## One-time installation

1. **Arduino IDE** — download from the "Software" section of arduino.cc.
2. **MiniCore board package** (per the Joy-it manual):
   - File → Preferences → Additional Board Manager URLs, paste
     `https://mcudude.github.io/MiniCore/package_MCUdude_MiniCore_index.json`
   - Tools → Board → Boards Manager → search **MiniCore** (by MCUdude) → Install.
3. **CH341 driver** — only if no COM port appears when the board is plugged in
   (Device Manager → "Ports (COM & LPT)" → "USB-SERIAL CH340"): install the
   **CH341SER** package from the Joy-it product page, replug.
   Reminder: this driver is for the *board's USB-C bridge*. The RC522 reader
   uses no driver at all — it is wired directly to the SPI pins.
4. **MFRC522 library** — Sketch → Include Library → Manage Libraries →
   search **MFRC522** (by GithubCommunity) → Install.

Do this on the reference laptop first, and on every laptop later (section A
step 10 of the plan).

## Tools settings — before EVERY upload

| Menu | Value |
|---|---|
| Tools → Board | MiniCore → **ATmega328** |
| Tools → Port | the COM port that appears when the board is plugged |
| Tools → Clock | **External 8 MHz** |
| Tools → Variant | **328PB** |

All other MiniCore menus (BOD, EEPROM, Reset…) stay at their defaults.
⚠ A sketch flashed with a wrong Clock runs at the wrong speed and the Serial
Monitor comes out garbled — if text looks wrong, this is the first place to
look.

## Verify (compile) — no board needed

- Open `..\src\animateur\01_Test_Nano\01_Test_Nano.ino` → Sketch → Verify → zero errors.
- Then Verify `..\src\enfants\02_MusiqueCartes_Nano\02_MusiqueCartes_Nano.ino` the
  same way. It compiles as shipped; its exercises are small marked gaps the
  kids fill during the session.

## First flash (with the board wired as in plan §A step 6)

- Upload `01_Test_Nano`. Expected: the DO-RE-MI scale on the buzzer, LED
  blinking at each note, Serial Monitor (115200) prints its banner.
- Rescue table — Upload fails to sync on a board sold without bootloader:
  Tools → Burn Bootloader once, with Programmer → **AVRISP mkII**, then Upload.

## After the build (plan §A step 9)

- Flash `02_MusiqueCartes_Nano`, write `DO` on a card with the phone, tap:
  the protocole tables are still EMPTY, so the expected answer is the sad
  refusal `Je ne connais pas "DO"... le protocole est-il casse ?` plus the
  sad double-bip — and that refusal is the proof that the whole reading
  chain works (phone → card → machine → screen + sound). For a true sound
  demo before the protocole day, flash the solved
  `..\src\animateur\03_MusiqueCartes_Nano_COMPLETE` instead: a `DO` card
  plays a real note; a `SOL4` card plays a long one (digit = duration;
  digit-less word = a noire).
- The sketch is then ready for the day: kids open the same folder, edit the
  contract tables in the marked exercise zones, and Upload again after each
  change.
