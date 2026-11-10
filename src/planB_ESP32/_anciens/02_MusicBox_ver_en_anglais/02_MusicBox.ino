/*
 * 02_MusicBox — the real machine.
 *
 * THE CONTRACT (both halves belong to the group):
 *
 *   Side A — written on the card, by the kid, with a phone (NFC Tools app:
 *            Write -> Add a record -> Text -> e.g. "DO").
 *   Side B — the tables below, in this file. The tuner edits them.
 *   Side C — the engine: this program reads the card's raw bytes, extracts
 *            the text (inside the phone's standard NDEF envelope), compares
 *            each word with the commands in the table, and plays the
 *            matching frequency.
 *
 * When side A and side B agree, the machine plays music.
 * When they disagree, it plays wrong or refuses — and the group must repair
 * the agreement. Breaking it on purpose (from either side!) is the lesson.
 *
 * Usage: tap a phone-written card on the reader. Every word found on the card
 * is played if it is in the table; unknown words print an error in the Serial
 * Monitor. Several words on one card = played one after the other, so a whole
 * melody fits on a single card.
 *
 * Wiring (ESP32 devkit):
 *   RC522: VCC 3.3V | GND | RST 21 | SDA/SS 5 | MOSI 23 | MISO 19 | SCK 18
 *   Buzzer (passive module, KY-006 type): S -> GPIO 25, middle -> 3.3V, - -> GND
 *   (2-pin bare passive buzzer instead: one leg -> GPIO 25, other -> GND)
 *   LED: GPIO 2 -> resistor 200-350 ohm (e.g. 220 or 330) -> LED -> GND
 *
 * Requires: "MFRC522" library by GithubCommunity + ESP32 core 3.x.
 */

#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN      5
#define RST_PIN     21
#define BUZZER_PIN  25
#define LED_PIN     2

// ===================== SIDE B: THE TABLES (KIDS EDIT HERE) =====================

// What text may appear on a card (upper or lower case, both accepted):
const char* NOTE_COMMANDS[] = {"DO", "RE",  "MI",  "FA",  "SOL", "LA",  "SI", "DO+"};
// What each command makes the buzzer do, in Hz (music is just numbers!):
const int   NOTE_FREQS[]    = { 262,  294,   330,   349,   393,   440,   494,  523 };
const int NOTE_COUNT   = 8;
const int NOTE_LENGTH_MS = 400;   // milliseconds per note

// ================================================================================

MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

// The card region a phone writes NDEF into: sectors 1 to 3 = blocks 4 to 15.
const byte FIRST_BLOCK = 4;
const byte LAST_BLOCK  = 15;
const int  MAX_BYTES   = 12 * 16; // the whole region, minus the key blocks

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  ledcAttach(BUZZER_PIN, 440, 8);

  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;  // factory key A

  SPI.begin();
  rfid.PCD_Init();
  Serial.println("\nMusicBox ready. Write text on a card with the phone "
                 "(NFC Tools), then tap it here.");
}

void loop() {
  if ( ! rfid.PICC_IsNewCardPresent() || ! rfid.PICC_ReadCardSerial()) return;

  byte dump[MAX_BYTES];
  int total = readCardRegion(dump);

  char text[MAX_BYTES + 1];
  int textLen = -1;
  if (total > 0) textLen = extractText(dump, total, text, MAX_BYTES);

  if (textLen <= 0) {
    Serial.println("No text found on this card. The phone must first write "
                   "one (NFC Tools: Write -> Text).");
    sadChirp();
  } else {
    playText(text);
  }

  rfid.PCD_StopCrypto1();
  rfid.PICC_HaltA();
  delay(500);                      // debounce: don't double-fire on one tap
}

// ---- Reader half of the engine ------------------------------------------------

// Reads the data blocks of sectors 1..3 (skips the trailer/key blocks).
// Returns the number of bytes read, or 0 if nothing could be read.
int readCardRegion(byte* out) {
  int total = 0;
  for (byte sector = 1; sector <= 3; sector++) {
    byte firstBlock = sector * 4;
    if (rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, firstBlock,
                              &key, &(rfid.uid)) != MFRC522::STATUS_OK)
      break;                         // this sector is sealed: skip it

    for (byte block = firstBlock; block < firstBlock + 4 && total < MAX_BYTES; block++) {
      if ((block % 4) == 3) continue;        // trailer block: keys, not data
      byte data[18];
      byte size = sizeof(data);
      if (rfid.MIFARE_Read(block, data, &size) == MFRC522::STATUS_OK) {
        memcpy(out + total, data, 16);
        total += 16;
      }
    }
  }
  return total;
}

// The phone hides the text inside an NDEF envelope. find out where the text
// starts, take it out of the envelope, and hand it to the table half below.
// Returns the text length, or -1 if no text could be extracted.
int extractText(const byte* raw, int rawLen, char* out, int outSize) {
  // 1) look for the NDEF start byte 0x03 written by the phone app
  int i = 0;
  while (i < rawLen && raw[i] != 0x03) i++;
  if (i >= rawLen - 2) return plainText(raw, rawLen, out, outSize); // no envelope found

  int msgLen = raw[i + 1];       // phones write the short form (< 255 bytes)
  if (msgLen == 0xFF || i + 2 + msgLen > rawLen)
    return plainText(raw, rawLen, out, outSize);                    // long form: fallback

  const byte* msg = raw + i + 2;

  // 2) walk the NDEF records inside the message until one is a text record
  int pos = 0;
  while (pos < msgLen) {
    byte header  = msg[pos];     pos++;
    byte typeLen = msg[pos];     pos++;
    if ( ! (header & 0x10))                  // long record: not covered here
      return plainText(raw, rawLen, out, outSize);
    byte payLen  = msg[pos];     pos++;
    byte idLen   = 0;
    if (header & 0x08) { idLen = msg[pos]; pos++; }
    if (pos + typeLen > msgLen) break;

    byte tnf = header & 0x07;
    const byte* payload = msg + pos + typeLen + idLen;

    if (tnf == 1 && typeLen >= 1 && msg[pos] == 'T') {   // a text record!
      byte langLen  = payload[0] & 0x3F;
      const byte* txt    = payload + 1 + langLen;
      int         txtLen = payLen - 1 - langLen;
      if (txtLen > outSize - 1) txtLen = outSize - 1;
      memcpy(out, txt, txtLen);
      out[txtLen] = '\0';
      return txtLen;
    }
    if ( ! (header & 0x40)) break;         // no "message end" flag: stop safely
    pos = (payload + payLen) - msg;        // next record
  }
  return plainText(raw, rawLen, out, outSize);
}

// Fallback when the envelope can't be decoded: keep every printable character.
// playText() then slices it into words, so leftover junk never gets played.
int plainText(const byte* raw, int rawLen, char* out, int outSize) {
  int o = 0;
  for (int i = 0; i < rawLen && o < outSize - 1; i++)
    if (raw[i] >= 0x20 && raw[i] <= 0x7E) out[o++] = (char)raw[i];
  out[o] = '\0';
  return o;
}

// ---- Table half of the engine: text -> sound ----------------------------------

// Slices the text into words, looks each word up in the table, plays the match.
void playText(const char* text) {
  int i = 0;
  bool playedAny = false;
  while (text[i] != '\0') {
    while (text[i] != '\0' && ! isLetter(text[i])) i++;   // skip what's not a word
    if (text[i] == '\0') break;
    int start = i;
    while ( isLetter(text[i])) i++;
    int wordLen = i - start;

    if (wordLen <= 15) {                 // longer words can't be a note command
      char word[16];
      for (int k = 0; k < wordLen; k++) word[k] = toUpper(text[start + k]);
      word[wordLen] = '\0';

      int note = -1;
      for (int n = 0; n < NOTE_COUNT; n++)
        if (strcmp(word, NOTE_COMMANDS[n]) == 0) note = n;

      if (note >= 0) {
        Serial.print("Playing: "); Serial.println(word);
        playNote(note);
        playedAny = true;
      } else if (wordLen >= 2) {         // single stray letters stay silent
        Serial.print("I do not know \""); Serial.print(word);
        Serial.println("\" - the contract is broken somewhere!");
        sadChirp();
      }
    }
  }
  if ( ! playedAny) Serial.println("Nothing playable on this card.");
}

bool isLetter(char c) {
  return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')
      || c == '+';                       // "+" makes part of "DO+" too
}

char toUpper(char c) {
  return (c >= 'a' && c <= 'z') ? (char)(c - 'a' + 'A') : c;
}

void playNote(int note) {
  digitalWrite(LED_PIN, HIGH);
  ledcWriteTone(BUZZER_PIN, NOTE_FREQS[note]);
  delay(NOTE_LENGTH_MS);
  ledcWriteTone(BUZZER_PIN, 0);
  digitalWrite(LED_PIN, LOW);
  delay(80);                             // tiny gap so separate notes stay separate
}

void sadChirp() {                        // audible "something is wrong"
  ledcWriteTone(BUZZER_PIN, 150); delay(150);
  ledcWriteTone(BUZZER_PIN, 100); delay(200);
  ledcWriteTone(BUZZER_PIN, 0);
}
