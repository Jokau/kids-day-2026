/*
 * 01_Test — étape 1 : vérifier les branchements (buzzer + LED seuls).
 * Pas de lecteur à ce stade. Si tu entends la mélodie et vois la LED
 * clignoter, les branchements sont bons.
 *
 * (Plan B : ce dossier ne sert QUE si les kits ESP32 arrivent à temps.
 *  La machine principale de l'atelier est 01_Test_Nano.)
 *
 * Carte visée : devkit ESP32 (USB-C + pont USB, cœur ESP32 3.x requis).
 * Branchement : buzzer S → GPIO 25, milieu → 3.3V, − → GND
 *               LED : GPIO 2 → résistance 200-350 ohm (ex : 220 ou 330) → LED → GND
 *
 * IDE : Tools → Board → « ESP32 Dev Module » + le port COM.
 * (Le cœur ESP32 3.x apporte la fonction ledcAttach.)
 */

#define BUZZER_PIN 25
#define LED_PIN    2

const int GAMME[] = {262, 294, 330, 349, 393, 440, 494, 523};  // DO RE MI FA SOL LA SI DO+

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  ledcAttach(BUZZER_PIN, 440, 8);      // le buzzer branché « en musique »

  Serial.println(F("Test du buzzer et de la LED..."));
}

void loop() {
  for (int i = 0; i < 8; i++) {
    digitalWrite(LED_PIN, HIGH);
    ledcWriteTone(BUZZER_PIN, GAMME[i]);   // le buzzer vibre a la frequence de la note
    delay(300);
  }
  ledcWriteTone(BUZZER_PIN, 0);        // le silence
  digitalWrite(LED_PIN, LOW);
  delay(1000);
}
