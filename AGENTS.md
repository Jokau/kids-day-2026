# ESP32 workshop folder — machine notes

A French 4-hour RFID music-box workshop for ~10-year-olds (Arduino Nano V4
3V + RC522 + buzzer + LED; ESP32 as plan B). All design reasoning lives in
`setup\SESSION_PLAN.md` — read it before proposing changes. This file only
records the environment facts and structure that are easy to get wrong.

## Environment facts (do not contradict these)

- **Git repo at the ESP32 root** (`C:\Users\Jokau\Documents\AI\ESP32\.git`,
  branch `master`, remote `origin` = git@github.com:Jokau/kids-day-2026.git).
  Before deleting or overwriting anything, read the target first.
- **PDF generation works and is installed**: use Microsoft Edge headless.
  This is how every `*.pdf` under `kid resources\` is produced from its
  sibling HTML source. **Regenerate the PDF immediately after every edit
  of its HTML source** — never leave a stale `*.pdf` on disk. Never say
  "there is no PDF plugin" — the pipeline
  is:
  ```
  "/c/Program Files (x86)/Microsoft/Edge/Application/msedge.exe" \
    --headless --disable-gpu --no-pdf-header-footer \
    --print-to-pdf="<absolute output path>" \
    "file:///C:/.../<html path with spaces URL-encoded>"
  ```
  (Folder names contain spaces — encode them in the `file://` URL.)
- **PDF *rendering* is not possible here**: no poppler/pdftoppm, so the
  Read tool cannot draw PDF pages. Say exactly that if asked to visually
  check a PDF; validate the HTML source instead, then regenerate.
- **No `arduino-cli`, no local C++ toolchain.** Sketches can only be truly
  compiled in the Arduino IDE (see `setup\ARDUINO-INIT.md`: MiniCore,
  ATmega328 / External 8 MHz / 328PB; ESP32 core 3.x+ for plan B). Local
  verification of sketch work = structural diff (below), never "it
  compiles" claims.
- **Python 3.14 available.** Run anything that reads project files with
  `python -I`. `setup\ecouter_chansons.py` plays audible beeps via
  `winsound` (Windows-only); for a silent dry run, monkeypatch it:
  `m.winsound.Beep = lambda f, d: None` after importing the module.

## Project structure

```
ESP32/
├─ AGENTS.md                         ← this file
├─ setup/                            ← the adult side
│  ├─ SESSION_PLAN.md                ← the session design: day script, wiring,
│  │                                    parts kit, timeline, checklists, extensions
│  ├─ ARDUINO-INIT.md                ← one-time IDE install + the four Tools
│  │                                    menu values needed before EVERY upload
│  ├─ ecouter_chansons.py            ← teacher's pre-flight ear check: interprets
│  │                                    the manual's card texts with the machine's
│  │                                    logic and beeps. Kept deliberately close
│  │                                    to its original — change it carefully and
│  │                                    only at the user's request
│  └─ __pycache__/                   ← ignore
├─ src/                              ← the sketches. src\LISEZMOI.md explains
│  ├─ animateur/                     ← ANIMATEUR side: the kids never open these
│  │  ├─ 01_Test_Nano/               ← wiring check (buzzer + LED only); flashed
│  │  │    01_Test_Nano.ino             by a helper, nobody edits it during the day
│  │  └─ 03_MusiqueCartes_Nano_COMPLETE/ ← the solved version = the official
│  │       03_MusiqueCartes_Nano_COMPLETE.ino working code of the animateur
│  │                                    (demo machine + answer key)
│  ├─ enfants/                       ← KIDS side: what each kit opens and edits
│  │  └─ 02_MusiqueCartes_Nano/      ← THE sketch the kids open and edit: EMPTY
│  │     02_MusiqueCartes_Nano.ino      protocole tables + three marked EXERCICE
│  │                                    zones (1: words + freqs to invent, 2:
│  │                                    their duration convention in
│  │                                    dureeEnDemiTemps(), 3: TEMPO_MS)
│  └─ planB_ESP32/                   ← the same machine on ESP32; used only if
│     ├─ 01_Test/                       those kits arrive. Same body logic, same
│     ├─ 02_MusiqueCartes/              empty-tables protocole, French docs;
│     ├─ 03_MusiqueCartes_COMPLETE/     uses ledcAttach()/ledcWriteTone() instead
│     └─ _anciens/                      of tone()/noTone()
│                                    ← superseded earlier drafts (an English
│                                       version, a dash-based duration
│                                       convention). Reference only; safe to
│                                       delete when the user asks
├─ kid resources/                    ← everything the kids receive or that is printed
│  ├─ manual/
│  │  ├─ manuel_enfant.html          ← the workshop booklet SOURCE (edit this)
│  │  └─ manuel_enfant.pdf           ← print-ready, generated from the HTML
│  └─ fiches/
│     ├─ fiches_atelier.html         ← print-ready cards: wiring cards 1-2,
│     └─ fiches_atelier.pdf             room-rules poster, kit checklist (A/B/C/D)
└─ resources/                        ← Joy-it hardware datasheet + manual (read-only)
```

## Invariants to keep when editing the sketches

- `02_MusiqueCartes_Nano` (kids) and `03_MusiqueCartes_Nano_COMPLETE`
  (solved) must differ only in the protocole zones: the two protocole table
  lines (kids': all-empty strings/zeros; solved: the demo encoding DO..DO+,
  RE+, VACHE), the three optional lines of `estMorceauDeMot()` (commented
  hint lines in the kids' file; active in the solved one — the demo
  protocole uses digits and symbols), and the `return` of
  `dureeEnDemiTemps()` (`return 0;` in the kids' empty function). Verify
  after any edit with `diff` of comment-stripped bodies (the header
  comments describing the diff are allowed to differ). The
  kids' file carries NO hint of any encoding: no suggested words, no
  suggested durations — the kids invent both, guided by the manual's
  crayon table and the song pages' scores. Exception: a group attempting
  the optional Défi 4 (secret-door fanfare, recipe in SESSION_PLAN) gains a
  fourth diff spot on its own laptop's copy — expected and un-doable.
- Their ESP32 twins hold the same invariant with only three classes of
  substitution: the pin defines (SS 5 / RST 21 / buzzer 25), `tone()`/`noTone()`
  → `ledcWriteTone()`, and the `pinMode(BUZZER_PIN, OUTPUT)` →
  `ledcAttach(BUZZER_PIN, 440, 8)` line. Everything else stays word-identical
  to the Nano versions.
- Card-text protocole logic (the fixed machinery the kids build on): whatever
  is written on a card is played in order; a line break is a one-beat pause;
  whatever is written behind a word — nothing (passed as 0) or trailing
  digits parsed without moving the text cursor — goes into the kids'
  `dureeEnDemiTemps()`, which must answer in **demi-temps**; a word that is
  only digits must skip cleanly, never hang the loop. WHAT the kids write
  and what each writing means is theirs to invent.
- RC522 keys (learned 2026-11 the hard way, cost a full day): a MIFARE
  sector's key depends on WHO wrote it — factory cards use
  `FFFFFFFFFFFF`; the **NFC Tools app re-locks every sector it writes with
  `D3F7D3F7D3F7`** (and MAD sector 0 with `A0A1A2A3A4A5`); other NFC
  writers use the NDEF standard `A0A1A2A3A4A5`. `lireLaCarte()` in all four
  sketches tries all three. **A failed authentication CLOSES the card**:
  every further auth times out until the card is woken (`PICC_WakeupA`) and
  re-selected (`PICC_Select`) — try the next key straight after a failure
  and even the right one fails. Wrong-key answers surface as *timeout*, not
  as a clean "auth error" — don't misread that as a wiring problem.
- Kid-facing text is French, without accents inside card-text examples
  (`RE`, not `RÉ`); serial messages avoid accents too.
