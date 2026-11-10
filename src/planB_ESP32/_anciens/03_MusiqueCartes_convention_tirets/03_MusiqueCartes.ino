/*
 * 03_MusiqueCartes — LA machine de l'atelier, la version des enfants.
 *
 * LE CONTRAT — c'est à VOUS de le décider, des DEUX côtés :
 *
 *   Moitié CARTE  (le·la musicien(ne), avec le téléphone NFC Tools) :
 *     quel mot écrire sur une carte pour dire quelle note ?
 *
 *   Moitié CODE   (le·la régleur(se), dans ce fichier) :
 *     quel mot fait jouer quelle fréquence ? Quelle durée pour quel symbole ?
 *
 * Les deux moitiés doivent dire la même chose. Si vous changez un côté,
 * l'autre devient faux : la machine joue de travers, et c'est à vous de
 * réparer le contrat.
 *
 * Dans ce fichier, trois petits exercices attendent l'équipe :
 *   EXERCICE 1 — les mots du contrat (les tableaux NOTE_COMMANDS/NOTE_FREQS)
 *   EXERCICE 2 — la convention de durée (fonction tempsEnDemiTemps)
 *   EXERCICE 3 — le tempo (TEMPO_MS : trop lent ? trop rapide ? à régler !)
 *
 * Utilisation : écris un mot sur une carte avec le téléphone (NFC Tools,
 * Write → Text), puis touche la carte au lecteur : la machine joue.
 * Une carte peut porter PLUSIEURS mots séparés par des espaces
 * (ex : "DO RE MI") : ils seront joués l'un après l'autre.
 *
 * Branchement (carte ESP32 classique) :
 *   RC522 : VCC 3.3V | GND | RST 21 | SDA/SS 5 | MOSI 23 | MISO 19 | SCK 18
 *   Buzzer passif (module KY-006) : S → GPIO 25, milieu → 3.3V, − → GND
 *   LED : GPIO 2 → résistance 200-350 ohm (ex : 220 ou 330) → LED → GND
 *
 * Bibliothèque : "MFRC522" de GithubCommunity + cœur ESP32 version 3.x.
 */

#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN      5
#define RST_PIN     21
#define BUZZER_PIN  25
#define LED_PIN     2

// =====================================================================
//             EXERCICE 1 — LE CONTRAT DES MOTS ET DES NOMBRES
// =====================================================================
//
// NOTE_COMMANDS = la liste des mots que la machine accepte sur une carte.
// NOTE_FREQS    = la fréquence (en hertz) jouée pour chaque mot.
//
// LA RÈGLE DU JEU : la liste ci-dessous est une PROPOSITION. Vous pouvez
// la remplacer par vos propres mots — des mots, des chiffres, des
// symboles — du moment que le téléphone et le code disent la même chose.
//
// Astuce : les lettres et les symboles + et - sont acceptés dans un mot.
// Écrivez sans accents (RE, pas RÉ) : le téléphone garde ses accents,
// la machine ne comprend que les lettres simples.
//
// De PLUS, les deux dernières cases sont VIDES : inventez-y votre 9e et
// 10e mot (une note aiguë ? un bruit rigolo ?) avec sa fréquence !

const char* NOTE_COMMANDS[10] = {"DO",  "RE", "MI", "FA", "SOL",
                                 "LA",  "SI", "DO+", "",    ""};

const int NOTE_FREQS[10] = { 262,  294, 330, 349, 393,
                             440,  494, 523,   0,   0 };  // 0 = encore vide

const int NOTE_COUNT    = 10;   // la taille des deux listes (8 vraies + 2 à vous)

const int TEMPO_MS = 500;       // EXERCICE 3 : la durée d'UN temps (une noire),
                                // en millisecondes. 500 = tranquille.
                                // Une mélodie pressée ? Essayez 300. Une
                                // berceuse ? Essayez 700. Réglez et écoutez !

// =====================================================================

MFRC522 rfid(SS_PIN, RST_PIN);          // notre lecteur de cartes
MFRC522::MIFARE_Key cle;                // la clé d'ouverture des cartes

// La zone de la carte que le téléphone utilise : secteurs 1 à 3,
// soit les blocs 4 à 15 (en sautant les blocs de clés).
const byte PREMIER_BLOC = 4;
const byte DERNIER_BLOC = 15;
const int  TAILLE_ZONE  = 12 * 16;      // 12 blocs de 16 caractères

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  ledcAttach(BUZZER_PIN, 440, 8);       // le buzzer est branché « en musique »

  for (byte i = 0; i < 6; i++) cle.keyByte[i] = 0xFF;   // la clé d'usine

  SPI.begin();
  rfid.PCD_Init();
  Serial.println();
  Serial.println("La machine attend une carte...");
  Serial.println("(ecris un mot avec le telephone : NFC Tools, Write, Text)");
}

void loop() {
  if ( ! rfid.PICC_IsNewCardPresent() || ! rfid.PICC_ReadCardSerial()) return;

  byte donnees[TAILLE_ZONE];            // ici on range TOUT ce que la carte dit
  int  total = lireLaCarte(donnees);

  char texte[TAILLE_ZONE + 1];          // ici on range le texte caché dedans
  int  longueur = -1;
  if (total > 0) longueur = extraireLeTexte(donnees, total, texte, TAILLE_ZONE);

  if (longueur <= 0) {
    Serial.println("Aucun texte sur cette carte.");
    Serial.println("(le telephone doit d'abord en ecrire un : NFC Tools, Write, Text)");
    bipTriste();
  } else {
    jouerLeTexte(texte);
  }

  rfid.PCD_StopCrypto1();
  rfid.PICC_HaltA();
  delay(500);                           // petite pause : sans elle, une même
                                        // carte serait lue deux fois de suite
}

// ---------------------------------------------------------------------
//  LIRE LA CARTE (la moitié « lecteur » de la machine)
// ---------------------------------------------------------------------

// Lit les blocs de données des secteurs 1 à 3 (les 4e..15e blocs, sans
// les blocs de clés). Renvoie le nombre de caractères lus, ou 0.
int lireLaCarte(byte* sortie) {
  int total = 0;
  for (byte secteur = 1; secteur <= 3; secteur++) {
    byte premierBloc = secteur * 4;
    // chaque secteur est un tiroir fermé à clé :
    if (rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, premierBloc,
                              &cle, &(rfid.uid)) != MFRC522::STATUS_OK)
      break;                            // tiroir verrouillé : on s'arrête

    for (byte bloc = premierBloc; bloc < premierBloc + 4 && total < TAILLE_ZONE; bloc++) {
      if ((bloc % 4) == 3) continue;    // bloc clé (tous les 4e) : on saute
      byte data[18];
      byte taille = sizeof(data);
      if (rfid.MIFARE_Read(bloc, data, &taille) == MFRC522::STATUS_OK) {
        memcpy(sortie + total, data, 16);
        total += 16;
      }
    }
  }
  return total;
}

// Le téléphone cache le texte dans une enveloppe bien rangée (format NDEF) :
// un repère de début, une longueur, le texte, et un code de langue
// (« en » ou « fr ») qui ne fait pas partie de TES mots.
// Notre métier : ouvrir l'enveloppe, sortir le texte propre.
// Renvoie la longueur du texte, ou -1 si rien n'a été trouvé.
int extraireLeTexte(const byte* brut, int brutLen, char* sortie, int sortieTaille) {
  // 1) chercher le repère d'enveloppe 0x03 posé par le téléphone
  int i = 0;
  while (i < brutLen && brut[i] != 0x03) i++;
  if (i >= brutLen - 2) return texteBrute(brut, brutLen, sortie, sortieTaille);

  int tailleMessage = brut[i + 1];      // les téléphones écrivent le format court
  if (tailleMessage == 0xFF || i + 2 + tailleMessage > brutLen)
    return texteBrute(brut, brutLen, sortie, sortieTaille);

  const byte* message = brut + i + 2;

  // 2) avancer d'enregistrement en enregistrement jusqu'au texte
  int pos = 0;
  while (pos < tailleMessage) {
    byte entete    = message[pos];  pos++;
    byte tailleType= message[pos];  pos++;
    if ( ! (entete & 0x10))          // format long : jamais sur une petite carte
      return texteBrute(brut, brutLen, sortie, sortieTaille);
    byte tailleTxt = message[pos];  pos++;
    byte tailleId  = 0;
    if (entete & 0x08) { tailleId = message[pos]; pos++; }
    if (pos + tailleType > tailleMessage) break;

    byte typeLequel = entete & 0x07;            // 1 = un enregistrement « texte »
    const byte* contenu = message + pos + tailleType + tailleId;

    if (typeLequel == 1 && tailleType >= 1 && message[pos] == 'T') {
      byte langueLen = contenu[0] & 0x3F;       // ex : 2 pour « en »
      const byte* mot   = contenu + 1 + langueLen;   // LE mot, enfin !
      int  motLen       = tailleTxt - 1 - langueLen;
      if (motLen > sortieTaille - 1) motLen = sortieTaille - 1;
      memcpy(sortie, mot, motLen);
      sortie[motLen] = '\0';
      return motLen;
    }
    if ( ! (entete & 0x40)) break;              // fin de message : stop net
    pos = (contenu + tailleTxt) - message;      // enregistrement suivant
  }
  return texteBrute(brut, brutLen, sortie, sortieTaille);
}

// Plan B : si l'enveloppe n'a pas été reconnue, on garde tous les
// caractères imprimables. Le découpage en mots plus bas ignorera le
// reste (les symboles bizarres ne font pas de son).
int texteBrute(const byte* brut, int brutLen, char* sortie, int sortieTaille) {
  int o = 0;
  for (int i = 0; i < brutLen && o < sortieTaille - 1; i++)
    if (brut[i] >= 0x20 && brut[i] <= 0x7E) sortie[o++] = (char)brut[i];
  sortie[o] = '\0';
  return o;
}

// ---------------------------------------------------------------------
//  JOUER LE TEXTE (la moitié « contrat » de la machine — c'est ICI que
//  vivent vos décisions !)
// ---------------------------------------------------------------------

bool estMorceauDeMot(char c) {
  return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')
      || c == '+'          // le « + » et le « ++ » : une octave au-dessus
      || c == '#'          // le « # » de FA#+ : la note noire
      || c == '-';        // le tiret : la durée de la note !
}

char enMajuscule(char c) {
  return (c >= 'a' && c <= 'z') ? (char)(c - 'a' + 'A') : c;
}

// Découpe le texte en mots, traduit chaque mot avec les tableaux, et joue.
// Chaque mot peut se terminer par des tirets : c'est la DURÉE de la note.
//    SOL     = une noire   (1 temps)
//    SOL-    = une croche  (un demi-temps)
//    SOL--   = une blanche (2 temps)
// (c'est la CONVENTION PROPOSÉE — votre équipe peut inventer la sienne,
//  à condition de la faire vivre des DEUX côtés !)
void jouerLeTexte(const char* texte) {
  int i = 0;
  bool quelqueChoseAJoue = false;
  while (texte[i] != '\0') {

    // sauter tout ce qui n'est pas un morceau de mot (espaces, virgules...)
    while (texte[i] != '\0' && ! estMorceauDeMot(texte[i])) i++;
    if (texte[i] == '\0') break;

    int debut = i;
    while (estMorceauDeMot(texte[i])) i++;
    int longueur = i - debut;

    // compter ET enlever les tirets de la fin : la durée, d'un côté,
    // le mot du contrat, de l'autre
    int tirets = 0;
    while (longueur > 0 && enMajuscule(texte[debut + longueur - 1]) == '-') {
      tirets++;
      longueur--;
    }

    if (longueur > 15 || longueur < 1) continue;    // ni trop long, ni vide

    char mot[16];
    for (int k = 0; k < longueur; k++) mot[k] = enMajuscule(texte[debut + k]);
    mot[longueur] = '\0';

    // chercher le mot dans le contrat
    int note = -1;
    for (int n = 0; n < NOTE_COUNT; n++)
      if (strcmp(mot, NOTE_COMMANDS[n]) == 0) note = n;

    if (note >= 0 && NOTE_FREQS[note] > 0) {
      int demisTemps = tempsEnDemiTemps(tirets);
      Serial.print("Je joue "); Serial.print(mot);
      Serial.print(" pendant ");
      if (demisTemps % 2 == 0) Serial.print(demisTemps / 2);
      else { Serial.print(demisTemps); Serial.print(" demi-temps"); }
      Serial.println(" temps");
      jouerUneNote(note, TEMPO_MS * demisTemps / 2);
      quelqueChoseAJoue = true;

    } else if (note >= 0) {             // le mot existe mais la case est vide
      Serial.print("Le mot "); Serial.print(mot);
      Serial.println(" est dans le contrat, mais sa fréquence n'est pas choisie !");
      bipTriste();

    } else if (longueur >= 2) {         // un vrai mot que le contrat ignore
      Serial.print("Je ne connais pas \""); Serial.print(mot);
      Serial.println("\"... le contrat est-il casse ?");
      bipTriste();
    }
    // (une lettre toute seule, sans tiret, ne dit rien et reste silencieuse)
  }
  if ( ! quelqueChoseAJoue) Serial.println("Rien de jouable sur cette carte.");
}

// ---------------------------------------------------------------------
//  EXERCICE 2 — LA CONVENTION DE DURÉE
// ---------------------------------------------------------------------
//
// Combien de temps dure un mot qui finit par tel nombre de tirets ?
// On répond en DEMI-TEMPS (pour pouvoir dire la croche, qui vaut un
// demi-temps, sans virgule).
//
//   PROPOSITION (à votre équipe de garder ou de changer !) :
//     0 tiret  → 2 demi-temps  = une noire   (1 temps)
//     1 tiret  → 1 demi-temps  = une croche  (un demi-temps)
//     2 tirets → 4 demi-temps  = une blanche (2 temps)
//     3 tirets → ??? la noire pointée (1 temps et demi) est proposée :
//                 à votre équipe de décider si vous la voulez !
//
int tempsEnDemiTemps(int tirets) {
  if (tirets == 0) return 2;    // le mot nu : une noire
  if (tirets == 1) return 1;    // un tiret : une croche
  if (tirets == 2) return 4;    // deux tirets : une blanche
  return 0;   // ← EXERCICE : 3 tirets = ? (indice : 1 temps et demi
              //               = trois demi-temps. Que renvoyer ?)
}

// ---------------------------------------------------------------------
//  FAIRE DU SON (le bout « fréquence → vibration »)
// ---------------------------------------------------------------------

void jouerUneNote(int note, int dureeMs) {
  digitalWrite(LED_PIN, HIGH);                // la lumière, en même temps
  ledcWriteTone(BUZZER_PIN, NOTE_FREQS[note]); // vibre à la fréquence du contrat
  delay(dureeMs);
  ledcWriteTone(BUZZER_PIN, 0);               // silence
  digitalWrite(LED_PIN, LOW);
  delay(80);                                  // un petit souffle entre les notes
}

void bipTriste() {                            // le son du « contrat cassé »
  ledcWriteTone(BUZZER_PIN, 150); delay(150);
  ledcWriteTone(BUZZER_PIN, 100); delay(200);
  ledcWriteTone(BUZZER_PIN, 0);
}
