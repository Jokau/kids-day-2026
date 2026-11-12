# RFID Music Box — 4h workshop (ages ~10; one kit = one machine)

**Primary board: the Arduino Nano V4 3V (Joy-it ARD-NanoV4-3V-MC).** It speaks
3.3 V logic as standard, matching the RC522 reader natively; it is powered by
USB-C at 5 V, and its pins lead out at the bottom, breadboard-friendly. The
ESP32 kits remain plan B at the end of the relevant sections, in case those
boards do arrive in time.

Each kit builds a machine that **plays music cards**. The kids themselves
write text onto blank cards with a phone (free "NFC Tools" app: Write → Add a
record → Text → for example `DO`), and then tap the card on the machine: the
machine reads the card, finds the text inside it, looks it up, and plays the
matching note.

What makes this workshop special is that **the kids build the entire protocole
between the card and the sound, with both halves in their own hands**: the
*written half* — which text goes on which card — is under their thumb on a
phone, and the *read half* — which text triggers which sound, and for how
long (the note durations: noire, blanche, croche) — is a set of tables and a
convention inside the program stored in the board's memory, which the kid
editing the code owns. The kids may choose words, numbers or symbols as they like — as long
as both halves say the same thing. When both halves agree, the machine plays
music. When one half is changed, the other one suddenly breaks: the child
then discovers that two halves of a protocole must always agree, and repairs it.

## Where everything lives in this project folder

- `setup\SESSION_PLAN.md` (this file) and `setup\ARDUINO-INIT.md` — the
  adult side: the day's script, the wiring cards, and the Arduino IDE
  installation sheet.
- `setup\ecouter_chansons.py` — a pre-flight ear check for the teacher: a
  small Python program that interprets the card texts of the kid manual with
  the machine's exact protocole logic (word splitting, trailing digits,
  half-beat durations) and beeps via Windows `winsound`; run it to hear every
  song of the manual — classics and the 2026 extra challenges (or
  `python ecouter_chansons.py "DO RE SOL||||"` for any own text) before
  printing the manual.
- `src\animateur\01_Test_Nano` — the test sketch: code that only *checks* a build
  (sound + LED). Flashed by a helper; nobody edits it during the day.
- `src\enfants\02_MusiqueCartes_Nano` — **the sketch the kids open, edit and
  upload**: their protocole lives in its marked exercise zones.
- `src\planB_ESP32\` — the same machine for an ESP32 board; nothing there
  is uploaded unless the ESP32 kits arrive.
- `kid resources\manual\` and `kid resources\fiches\` — everything the kids
  receive or that is printed for them: the workshop booklet (print-ready PDF
  plus its editable HTML source) and the print-ready cards (wiring cards,
  wiring cards, room-rules poster, kit checklist).
- `resources\` — hardware reference material (the Joy-it datasheet and
  manual).

The sketches the day meets, in order — Nano (primary):

- `src\animateur\01_Test_Nano` — the wiring check (sound + LED only).
- `src\enfants\02_MusiqueCartes_Nano` — the machine the kids work in: comments in
  French, and three small marked exercises (the protocole
  words, the duration convention, the tempo). It runs once the kids fill
  it: the two tables of Exercise 1 are EMPTY (their own words, their own
  frequencies), the duration function is EMPTY (their own convention), and
  only `TEMPO_MS` ships with a starting value (500).
- `src\animateur\03_MusiqueCartes_Nano_COMPLETE` — the same machine with every
  exercise **solved**: this is the teacher's demonstration machine (played
  at the introduction) and the answer key, kept separate from what the kids
  open. Comparing the two files shows exactly three solved spots: the two
  protocole tables (empty in the kids' file, filled with the demo encoding in
  this one) and the duration function. Nothing else.

ESP32 plan B — the same machine for a different board (only if the ESP32
kits arrive): `src\planB_ESP32\01_Test` (wiring check), `02_MusiqueCartes`
(the kids' version, with the same three exercises) and
`03_MusiqueCartes_COMPLETE` (the teacher's demonstration machine).
(The subfolder `_anciens\` keeps superseded earlier versions — an English
draft and an old dash-based duration convention — for reference only.)

## Parts per kit (one kit = one machine)

| Part | Qty | Notes |
|---|---|---|
| **Arduino Nano V4 3V** (Joy-it ARD-NanoV4-3V-MC) | 1 | 3.3 V logic as standard (its solder bridge stays untouched), USB-C socket, ATmega328PB. Its USB bridge is CH341-family, so the only USB driver involved is the CH341 one. (Plan B: an ESP32 devkit with USB-C + CP2102.) |
| RC522 RFID reader | 1 | This reader communicates over SPI, which is why it needs seven wires. See the wiring section below. |
| RFID cards or keyfobs, **MIFARE Classic 1K** | 8 | The phone must be able to *write* data into the card's memory, so the cards must be genuine **MIFARE Classic** (13.56 MHz). NTAG or Ultralight cards, and UID-changeable ("magic") cards, do not accept these writes. Fresh, never phone-formatted cards are easiest: the NFC Tools app formats a fresh card itself on first write. |
| Android or iOS phone **with NFC**, one per kit or one shared per two kits | 1 | The free **NFC Tools** app writes the kids' text onto the cards. No account and no internet needed; install it before the day. Nearly all mid-range and recent phones have NFC — check for the "N" setting under Settings → Connected devices. |
| Passive buzzer module, 3-pin (KY-006 type or equivalent) | 1 | Must be **passive** (see below). Wiring: S→D8, middle→3V3, −→GND. Budget about 0,50 € each; buy 5 including one spare. **How to recognise the right part:** the word "active" anywhere on the listing means the buzzer makes one fixed tone and cannot play a melody — reject it. A "16R/16Ω" coil is acceptable **on a 3-pin module** that carries its own transistor, because the module does the current work. A bare two-leg 16Ω part must not be connected directly to the board, because it would overload the pin (it would need an extra S8050 transistor plus a resistor). Anything marked 12V or sold as an SMD chip (for example the MLT-8530, which has no legs at all) is not usable on a breadboard. |
| LED + one 200–350 Ω resistor (220 or 330, both fine — it only sets the LED's brightness, not anything else in the machine) | 1 | The LED lights up while a note is playing. |
| Breadboard and dupont wires | — | About 12 dupont wires per kit are needed for all connections. |

## Wiring — Arduino Nano V4 3V (print one wiring card per kit from
`kid resources\fiches\fiches_atelier.html` — same tables as below)

The RC522 reader connects to the Nano with seven wires, as follows:
`RC522 VCC → Nano +3V3` · `RC522 GND → Nano GND` · `RC522 RST → Nano D9` ·
`RC522 SDA → Nano D10` · `RC522 MOSI → Nano D11` · `RC522 MISO → Nano D12` ·
`RC522 SCK → Nano D13`. The reader's `IRQ` pin stays unconnected.

**One driver clarification:** the RC522 needs *no* driver at all — it is wired
directly onto the board's SPI pins; nothing about it plugs into USB. The only
USB driver of this workshop is the CH341-family driver for the board's own
USB-C bridge (step 5 of section A).

The buzzer module connects with three wires: `S → D8`, `middle pin → 3V3`,
`− → GND`. (If a kit uses a bare two-leg passive buzzer instead, connect
one leg to D8 and the other to GND.)

The LED connects through its resistor: `D2 → resistor (200–350 Ω, e.g. 220) → LED → GND`. Red, yellow or green LEDs light clearly on 3.3 V with any value in this range; a blue or white LED would need the smallest value available (≈150 Ω) and still stay modest.

**On the Nano, the solder bridge stays untouched:** it is factory-set to
3.3 V, which is exactly what this project needs. Nobody — child or helper —
opens it during the day.

**Powering the board.** The board has a single USB-C connector, and it is the
only place where power enters the board. Plug one end of the USB-C cable into
this connector. The other end goes either to a laptop, which provides power
*and* allows programming and reading the Serial Monitor, or to any standard 5V
USB phone charger, which provides power only but is entirely sufficient to run
the machine. **Safety rule while wiring:** whenever a wire must be moved or
checked, first unplug the USB-C cable, then move or check the wire, and only
then reconnect the USB-C cable.

**Troubleshooting.** If the sketch reports that it cannot find the reader, then
in about nine cases out of ten the cause is one of two classic mistakes: either
the 3.3 V and GND wires are swapped, or the SDA and RST wires are swapped. Check
both the physical wires on the breadboard and the pin numbers in the code.

### Plan B — the same wiring on an ESP32 (only if it arrives)

`RC522 VCC → ESP32 3.3V` · `GND → GND` · `RST → pin 21` · `SDA → pin 5` ·
`MOSI → pin 23` · `MISO → pin 19` · `SCK → pin 18`; buzzer `S → pin 25`,
middle → 3.3V, `− → GND`; LED `pin 2 → resistor → LED → GND`. Then use the
sketches of the `src\planB_ESP32` folder. For the IDE: install "esp32" by
Espressif Systems version 3.x or newer through Boards Manager (URL:
`https://espressif.github.io/arduino-esp32/package_esp32_index.json`), select
Board "ESP32 Dev Module" + the COM port; if no COM port appears, install the
Silicon Labs CP210x VCP driver; if Upload fails with "Failed to connect", hold
the board's BOOT button while Upload runs. These boards use the `ledcAttach`
function, which requires core 3.x.

## Before the workshop day (teacher only)

- Install the Arduino IDE, available from the "Software" section of arduino.cc.
- Install the **MiniCore** board package for the Nano (per the manufacturer's
  manual): paste `https://mcudude.github.io/MiniCore/package_MCUdude_MiniCore_index.json`
  into File → Preferences → Additional Board Manager URLs, then install
  "MiniCore" by MCUdude through Tools → Board → Boards Manager.
- Install the **CH341 driver** for the board's USB bridge if Windows does not
  create a COM port by itself: the CH341SER package is downloadable from the
  Joy-it product page. (Once more: this driver is for the *board's USB
  connector*, not for the RC522, which needs no driver.)
- Install the **MFRC522** library (by GithubCommunity) through Sketch →
  Include Library → Manage Libraries.
- Install all of the above on one machine, and flash both sketches once on
  that machine so that everything is proven working before the day.
- Install the **NFC Tools** app on every phone that will be used, and test one
  full write on a real card: open NFC Tools → WRITE → Add a record → Text, type
  `DO`, tap the card to the phone until writing finishes, then check under
  READ that the card now contains the text.
- Check that each kit's fresh cards really are blank and writable: write any
  text onto one card per pouch, then erase it with NFC Tools (Other → Format /
  Erase); if a card refuses the write or the format, it is the wrong card type.
- Prepare about 30 blank MIFARE Classic cards, packed as one pouch of 8 cards
  per kit.

## Step-by-step implementation plan

### A. Before the day — build one validated reference kit

1. **Install the Arduino IDE**, downloadable from the "Software" section of
   arduino.cc.
2. **Add the MiniCore board package** (per the manufacturer's manual): in
   File → Preferences, paste
   `https://mcudude.github.io/MiniCore/package_MCUdude_MiniCore_index.json`
   into "Additional Board Manager URLs", then install "MiniCore" by MCUdude in
   Boards Manager. For every upload, set the four Tools entries:
   Board → MiniCore → **ATmega328** · Port → the board's COM port ·
   **Clock → External 8 MHz** · **Variant → 328PB**. Leave all other MiniCore
   menus (BOD, EEPROM, Reset…) at their defaults. If a first Upload ever
   fails with no sync (a board sold without its bootloader), run Tools →
   **Burn Bootloader** once, with Programmer → **AVRISP mkII** as the manual
   specifies, then Upload.
3. **Install the library.** In Arduino IDE, open Sketch → Include Library →
   Manage Libraries, search for **MFRC522** (by GithubCommunity) and install it.
4. **Compile check.** Open the folder `src\animateur\01_Test_Nano` in
   Arduino IDE and run Sketch → Verify. It must compile with zero errors.
   Then verify `src\enfants\02_MusiqueCartes_Nano` (the sketch the
   kids work in) the same way. It compiles and runs as shipped: its protocole
   tables are EMPTY (the kids fill them with their own words and frequencies)
   and the duration function is EMPTY
   (the "what does the kid's convention return"
   exercise, the tempo). Any problem found here is cheap to fix; the same
   problem on the workshop day is not.
5. **Flash `01_Test_Nano`.** With the four Tools entries of step 2 set,
   click Upload. Open the Serial Monitor at **115200**. The board's USB-C
   bridge is CH341-family: if no COM port appears under Device Manager →
   "Ports (COM & LPT)" on Windows, install the **CH341SER** driver from the
   Joy-it product page and replug the board. No BOOT-button trick is needed
   on this board; if the Serial Monitor shows garbled text or nothing runs,
   check that Tools → Clock really is "External 8 MHz" — a sketch flashed at
   the wrong clock runs at the wrong speed.
6. **Wire the sound and the light.** Connect the buzzer module (`S → D8`,
   `middle → 3V3`, `− → GND`) and the LED (`D2 → 220–330 Ω → LED → GND`), then
   press the board's Reset button. You must hear the melody DO-RE-MI… to the DO an octave above, and
   see the LED blink in sync. If instead you hear a single flat buzz no matter
   what the program plays, the buzzer is an active model: replace the part with
   a passive buzzer module.
7. **Wire the reader — with power off.** Unplug the USB cable, connect the
   seven RC522 wires exactly as listed on the wiring card, and only then plug
   the USB cable back in. The reader must never be plugged or unplugged while
   the board is powered.
8. **Verify the cards.** Flash the example sketch `File → Examples → MFRC522 →
   DumpInfo`, tap one card taken **from each batch of cards**, and read its
   report in the Serial Monitor. Every batch must report `PICC type: MIFARE
   1KB` for its cards to be usable.
   Also run the ear check: `python setup\ecouter_chansons.py` plays every
   song of the kid manual exactly as the machine will interpret them — adjust
   the manual's sequences if something sounds wrong while there is still
   time, not on the day.
9. **Flash `02_MusiqueCartes_Nano` and run the first full round trip.** With the
   phone, write the text `DO` onto one blank card (NFC Tools: Write → Add a
   record → Text). Tap that card on the reader: the sketch's protocole is
   still EMPTY, so the honest expected answer is the sad refusal
   `Je ne connais pas "DO"... le protocole est-il casse ?` plus the sad
   double-bip — that refusal IS the proof that the whole reading chain works:
   phone → card → machine → screen + sound. (The teacher's demo machine,
   running the solved sketch, plays `DO` for real whenever a true
   end-to-end sound demonstration is wanted before the protocole day.)
10. **Freeze the reference kit.** Photograph the wired kit exactly as it now
    stands, print one wiring card per kit, prepare one pouch of 8 fresh blank
    cards per kit, and copy the sketch folder `src\enfants\02_MusiqueCartes_Nano`
    onto every laptop, with the MiniCore setup already done on each one (the
    helper's laptop also keeps `src\animateur\01_Test_Nano`). The teacher's
    demonstration machine flashes `src\animateur\03_MusiqueCartes_Nano_COMPLETE`.

### B. On the day — the same sequence, repeated at each kit

**Table kit checklist (per kit).** Every item on this list must be on the
table before the build starts; a single missing item stalls the build.
Anything missing later must be looked for here first.

- The Arduino Nano V4 3V board
- The RC522 RFID reader
- The KY-006 passive buzzer module
- One LED and one resistor in the 200–350 Ω range
- One breadboard
- About 12 dupont wires, **color-coded in advance**: red for the power wires,
  black for the ground wires, and a few other colors for the signal wires —
  the printed wiring card suggests one color per pin, so that a color is a
  constant aid during the whole day
- One pouch of 8 fresh blank MIFARE cards
- A phone with the NFC Tools app installed (one per kit, or shared between
  two neighbouring kits)
- One printed wiring card
- One marker, to label each card once its text is written
- One USB-C **data** cable plus one 5V USB phone charger
- Access to a laptop — needed only at flashing moments; one or two shared
  laptops are sufficient for the whole room

**Room rules, printed and displayed where every child can see them:**

- *"USB out = wire authorised. USB in = hands off."* In other words: no wire
  may be moved or checked while the board is powered, because a carelessly
  fumbled wire on a live board is how readers get damaged.
- The USB-C cable goes into the board's single USB-C connector; either a laptop
  or a phone charger can be at the other end, as both provide the same 5V.

1. **0:20–1:00 — Unpack, then wire the sound and the light.** Unpack the
   kit, name the parts aloud, then wire the LED and the buzzer only,
   following the printed wiring card; ready the laptop meanwhile.
2. **Checkpoint 1.** A helper flashes `01_Test_Nano`. The kit must hear the scale
   melody. If there is no sound, check the two classic faults first: the
   buzzer's S wire is on the wrong pin, or the buzzer's middle pin went to 5V
   instead of 3.3V. (For a bare two-leg buzzer, the equivalent fault is the two
   legs in each other's row.)
3. **1:00–1:45 — Wire the reader.** The USB cable is unplugged. Connect the
   seven RC522 wires one wire at a time, checking each wire against the
   wiring card as it is placed. Only when all seven wires have
   been checked is the USB cable plugged back in.
4. **Checkpoint 2.** A helper flashes `02_MusiqueCartes_Nano`. The Serial Monitor
   must print "La machine attend une carte...". A quick first test: tap a card
   that the teacher has already written — it must play. If the machine answers
   "Aucun texte sur cette carte." on a correctly written card, then in most
   cases the RC522's VCC and GND wires, or its SDA and RST wires, are swapped —
   see the troubleshooting note above.
5. **2:00–2:45 — Invent the protocole, then write both halves.** First
   *decide* the encoding, for **both** the notes and their **durations**:
   which word goes on a card for each note — `DO`? lowercase `do`? a number? a
   symbol? — and how the duration (noire, blanche, croche) gets written. The
   sketch suggests NOTHING: its tables and its duration function are empty;
   the only mechanics it fixes is that anything written behind a word is
   passed to `dureeEnDemiTemps()`, which must answer in **demi-temps** (the
   croche = one demi-temps is the reason). Fill the crayon table first (8
   notes: word + frequency from the hertz grid, rounded), then apply the
   decision: fill the three marked exercises of the sketch, with a helper
   flashing the result — and write the exact cards with the phone, one word
   per card, one card at a time on NFC Tools. Both halves being done, tap card
   after card and hear the protocole play. Then comes the
   moment that gives the lesson its meaning: **break the machine on purpose,
   once from each side.** Change ONE frequency in the table (a helper flashes
   it): tap a card — it plays a *wrong note*: the machine is not broken, it
   has merely been retuned. Then repair the table. Next, write a word the
   table does not know:
   the machine refuses with `Je ne connais pas "XX"... le protocole est-il
   casse ?`, and then choose: extend the table, or rewrite the card. Two
   halves, two repair directions — one protocole.
6. **2:45–3:30 — Compose and rehearse.** Each kit now builds a melody —
   its own, or one of the ready-made scores printed in the kid manual: the
   classics (Au clair de la lune, Frère
   Jacques, Ah ! vous dirai-je maman, Ode à la joie, Vive le vent, and a
   simplified Joyeux anniversaire — which sings a note above the top DO (the
   "RE aigu"), so a group choosing it must enlarge its protocole first: fill
   one of the two free table slots, word + frequency 587–588) plus the 2026 extra challenges
   block (the full Golden chorus and the APT. rhythm hook). The rule of the
   cards is the simplest possible: **whatever is written on a card is what
   the machine plays, in order** — one word, several words, or the whole
   song, as the group likes; a new line on the card is a small breath (a
   one-beat pause between phrases). The score is written with the
   phone onto card(s), kept in playing order on the table, and
   each card is labeled with the marker. Each kit sets its own `TEMPO_MS`
   and rehearses: long notes for the blanches, short ones for the croches.
   To perform, one child taps the reader with the cards one after the other:
   each tap lights the LED and plays everything the tapped card says.
7. **3:30–4:00 — Concert and extensions.** Every melody gets its concert
   moment. If time remains, the kits take up the extension challenges below.

## Timeline

Agenda view only; the operational detail is in section B of the plan above.

| Time | Phase |
|---|---|
| 0:00–0:20 | Introduction. The teacher shows a finished machine performing. Everyone identifies all the parts on the table. The day is simply a sequence of steps — wiring, then code, then cards — done in that order, however the room is organised.  The teacher's demo machine runs the solved sketch `src\animateur\03_MusiqueCartes_Nano_COMPLETE`. |
| 0:20–1:00 | Wire the sound and the light, then reach checkpoint 1: the kit hears the scale melody. |
| 1:00–1:45 | Wire the seven reader wires with power off, then reach checkpoint 2: the Serial Monitor prints "La machine attend une carte...". |
| 1:45–2:00 | Break. Take a photo of each kit at its checkpoint state. |
| 2:00–2:45 | The protocole. The encoding is invented — the words *and* the duration marks — then applied to both halves (the code's tables and duration function, the phone-written cards), then deliberately breaks the machine from each side and repairs both. |
| 2:45–3:30 | Compose and rehearse. A song is chosen (one of the ready-made scores in the kid manual — classics or the 2026 extra challenges — or an original), the score is written onto cards (one card may carry the whole song), `TEMPO_MS` is set, and the playing is rehearsed. |
| 3:30–4:00 | Every melody gets performed, then the extension challenges open up. |

## Extension challenges (for early finishers)

1. **Octave up.** Add four higher notes to the two tables: doubling any
   frequency produces the same note one octave higher.
2. **Rhythm.** Change `TEMPO_MS` to make each song slower or faster, and hear
   how this changes a melody's character.
3. **Jukebox.** Write a whole song on a single card (even all four lines of
   Joyeux anniversaire fit — a new line for each line of the score), label
   it with the marker as a title, and keep going: the builder ends with a
   stack of cards, each card a song — a playlist, played on demand.
   (Already works out of the box; the challenge is the phone-writing skill
   it demands.)
4. **Secret door.** Ready-made recipe (copy-paste file for the animateur:
   `src\animateur\DEFI_4_fanfare.txt`): with the group, first fill a free
   table slot (9 or 10) with their chosen word + frequency (the "trigger
   word"). Then, in `02_MusiqueCartes_Nano`, find in `jouerLeTexte` the line
   `jouerUneNote(note, TEMPO_MS * demisTemps / 2);` and replace it with:

   ```cpp
      if (note == 8) {                             // le mot de la case 9 !
        jouerUneNote(note, TEMPO_MS);              // 1re note de la fanfare (la sienne)
        jouerUneNote(4, TEMPO_MS);                 // 2e note : change 4 (un index !) pour celle que tu veux
        quelqueChoseAJoue = true;
      } else {
        jouerUneNote(note, TEMPO_MS * demisTemps / 2);
        quelqueChoseAJoue = true;
      }
   ```

   The two indices are kid-decisions: `8` = which table slot is the secret
   door (0-based: case 1 is 0), `4` = which note the fanfare ends on. Tap
   the trigger card: its own note rings, then the chosen note — a fanfare.
   Note: this adds a fourth kids-vs-correction diff spot on that group's
   laptop only; if the group wants the pristine file back afterwards, just
   delete the `if/else` and keep the single original line.

## Why nothing is "stored" in the board

A MIDI note is just a frequency — a number of vibrations per second. The
board's hardware (the timer that drives `tone()`) makes the buzzer pin vibrate
at exactly that frequency, so every sound the machine makes is **synthesised
live**. No
audio files and no SD card are needed anywhere in this project. (Audio files
would only matter for recorded sounds or real songs; playing those requires a
DFPlayer Mini board, which is the natural follow-up kit for a later session.)
