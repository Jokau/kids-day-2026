/*
 * 01_Test_Nano — étape 1 : vérifier les branchements (buzzer + LED seuls).
 * Pas de lecteur à ce stade. Si tu entends la mélodie et vois la LED
 * clignoter, les branchements sont bons.
 *
 * Carte visée : Arduino Nano V4 3V (Joy-it ARD-NanoV4-3V-MC, logique 3.3 V).
 * Branchement : buzzer S → D8, milieu → +3V3, − → GND
 *               LED : D2 → résistance 200-350 ohm (ex : 220 ou 330) → LED → GND
 *
 * Nécessite dans l'IDE (voir SESSION_PLAN.md) :
 *   le paquet « MiniCore » installé, puis :
 *   Tools → Board → MiniCore → ATmega328
 *   Tools → Port   → le port COM de la carte
 *   Tools → Clock  → External 8 MHz
 *   Tools → Variant→ 328PB
 */

#define BUZZER_PIN 8
#define LED_PIN    2

const int GAMME[] = {262, 294, 330, 349, 393, 440, 494, 523};  // DO RE MI FA SOL LA SI DO+

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  Serial.println(F("Test du buzzer et de la LED..."));
}

void loop() {
  for (int i = 0; i < 8; i++) {
    digitalWrite(LED_PIN, HIGH);
    tone(BUZZER_PIN, GAMME[i]);       // le buzzer vibre a la frequence de la note
    delay(300);
  }
  noTone(BUZZER_PIN);                 // le silence
  digitalWrite(LED_PIN, LOW);
  delay(1000);
}
