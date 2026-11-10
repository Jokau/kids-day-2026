/*
 * 02_MusiqueCartes_Nano — LA machine de l'atelier, version carte Arduino Nano.
 *
 * Carte visée : Arduino Nano V4 3V (Joy-it ARD-NanoV4-3V-MC, logique 3.3 V).
 *
 * LE PROTOCOLE — c'est à TOI de le décider, des DEUX côtés :
 *
 *   Côté CARTE   (avec le téléphone NFC Tools) :
 *     quel mot écrire sur une carte pour dire quelle note ?
 *
 *   Côté CODE    (dans ce fichier) :
 *     quel mot fait jouer quelle fréquence ? Quelle durée pour quel symbole ?
 *
 * Les deux moitiés doivent dire la même chose. Si tu changes un côté,
 * l'autre devient faux : la machine joue de travers, et c'est à toi de
 * réparer le protocole.
 *
 * Dans ce fichier, trois petits exercices t'attendent :
 *   EXERCICE 1 — les mots du protocole (les deux tableaux, VIDES au départ :
 *                à toi d'y écrire tes mots et tes fréquences)
 *   EXERCICE 2 — ta convention de durée (fonction dureeEnDemiTemps)
 *   EXERCICE 3 — le tempo (TEMPO_MS : trop lent ? trop rapide ? à régler !)
 *
 * Utilisation : écris ce que tu veux sur une carte avec le téléphone (NFC
 * Tools, Write → Text), puis touche la carte au lecteur : la machine joue
 * TOUT ce qui est écrit dessus, dans l'ordre — un mot, plusieurs mots
 * (ex : "DO RE MI"), ou la chanson entière. Un saut de ligne sur la carte
 * vaut une petite pause d'un temps (la respiration entre deux phrases).
 *
 * Branchement sur la Nano V4 3V :
 *   RC522 : VCC → +3V3 | GND → GND | RST → D9 | SDA/SS → D10
 *           MOSI → D11 | MISO → D12 | SCK → D13      (IRQ : pas de fil)
 *   Buzzer passif (module KY-006) : S → D8, milieu → +3V3, − → GND
 *   LED : D2 → résistance 200-350 ohm (ex : 220 ou 330) → LED → GND
 *   Alimentation : USB-C (5 V) — ordinateur ou chargeur.
 *
 * Nécessite : bibliothèque « MFRC522 » (GithubCommunity) + paquet MiniCore
 * (Tools → Board → MiniCore → ATmega328 ; Clock → External 8 MHz ; Variant
 * → 328PB). À la différence de sa version plan B ESP32
 * (src\planB_ESP32\02_MusiqueCartes), ce fichier utilise tone()/noTone()
 * et n'a pas besoin du cœur ESP32.
 */

#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN       10     // broche SDA/SS du lecteur
#define RST_PIN       9     // broche RST du lecteur
#define BUZZER_PIN    8
#define LED_PIN       2

// =====================================================================
//        EXERCICE 1 — LE PROTOCOLE DES MOTS ET DES NOMBRES
// =====================================================================
//
// NOTE_COMMANDS = TES mots — un mot, une note. La case 1 = ta carte n°1
//                (la note DO du tableau au crayon du manuel), la case 8 =
//                la note la plus aiguë, les cases 9 et 10 = tes inventions.
// NOTE_FREQS    = les fréquences (en hertz) de ton tableau du manuel,
//                recopiées à la MÊME POSITION que chaque mot.
//
// LA RÈGLE DU JEU : les deux tableaux du code sont VIDES — c'est TOI qui
// y écris ton protocole : un mot par note, et SA fréquence en dessous,
// dans le même ordre que ton tableau au crayon. Le mot écrit sur la carte
// doit être exactement celui du tableau : un protocole vit des DEUX côtés !
//
// Astuce : un mot peut être fait de lettres, de chiffres ou de symboles
// (le +, le -, ce que tu veux) — MAIS écris sans accents : le téléphone
// garde ses accents, la machine ne comprend que les lettres simples.

const char* NOTE_COMMANDS[10] = {"",    "",    "",   "",   "",
                                 "",    "",    "",   "",   ""};

const int NOTE_FREQS[10] = { 0, 0, 0, 0, 0,
                             0, 0, 0, 0, 0 };   // 0 = pas encore choisi

const int NOTE_COUNT    = 10;   // la taille des deux listes (8 notes + 2 à toi)

const int TEMPO_MS = 500;       // EXERCICE 3 : la durée d'UN temps (une noire),
                                // en millisecondes. 500 = tranquille.
                                // Une mélodie pressée ? Essayez 300. Une
                                // berceuse ? Essayez 700. Réglez et écoutez !

// =====================================================================

MFRC522 rfid(SS_PIN, RST_PIN);          // notre lecteur de cartes
MFRC522::MIFARE_Key cle;                // la clé d'ouverture des cartes

// La mémoire de travail de la lecture. Sur cette carte, la mémoire vive
// (SRAM) est petite — 2 kilo-octets ! — donc on réserve ces tables UNE
// seule fois, ici, pour toujours (et non à chaque lecture de carte).
const byte PREMIER_BLOC = 4;            // le téléphone écrit à partir du bloc 4
const byte DERNIER_BLOC = 15;
const int  TAILLE_ZONE  = 12 * 16;      // 12 blocs de 16 caractères

byte donneesCarte[TAILLE_ZONE];         // ici on range TOUT ce que la carte dit
char texteCarte[TAILLE_ZONE + 1];       // ici on range le texte caché dedans

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  for (byte i = 0; i < 6; i++) cle.keyByte[i] = 0xFF;   // la clé d'usine

  SPI.begin();
  rfid.PCD_Init();
  Serial.println();
  Serial.println(F("La machine attend une carte..."));
  Serial.println(F("(ecris un mot avec le telephone : NFC Tools, Write, Text)"));
}

void loop() {
  if ( ! rfid.PICC_IsNewCardPresent() || ! rfid.PICC_ReadCardSerial()) return;

  int total = lireLaCarte(donneesCarte);

  int longueur = -1;
  if (total > 0)
    longueur = extraireLeTexte(donneesCarte, total, texteCarte, TAILLE_ZONE);

  if (longueur <= 0) {
    Serial.println(F("Aucun texte sur cette carte."));
    Serial.println(F("(le telephone doit d'abord en ecrire un : NFC Tools, Write, Text)"));
    bipTriste();
  } else {
    jouerLeTexte(texteCarte);
  }

  rfid.PCD_StopCrypto1();
  rfid.PICC_HaltA();
  delay(500);                           // petite pause : sans elle, une même
                                        // carte serait lue deux fois de suite
}

// ---------------------------------------------------------------------
//  LIRE LA CARTE (la moitié « lecteur » de la machine)
// ---------------------------------------------------------------------

// Lit les blocs de données des secteurs 1 à 3 (les blocs 4..15, sans
// les blocs de clés). Renvoie le nombre de caractères lus, ou 0.
// Chaque secteur est un tiroir fermé à clé — la clé dépend de QUI a écrit
// la carte : usine (carte vierge), clé « D3F7 » de l'appli NFC Tools, ou
// clé « A0A1 » du standard NDEF. On essaie les trois, l'une après l'autre.
// Et après un essai raté la carte se REFERME : il faut la réveiller et la
// resélectionner avant l'essai suivant — sinon même la bonne clé échoue
// (une carte refermée ne répond plus qu'avec un « timeout »).
int lireLaCarte(byte* sortie) {
  MFRC522::MIFARE_Key cleNFC  = {0xD3, 0xF7, 0xD3, 0xF7, 0xD3, 0xF7};
  MFRC522::MIFARE_Key cleNdef = {0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5};
  MFRC522::MIFARE_Key* lesCles[3] = { &cle, &cleNFC, &cleNdef };
  int total = 0;
  for (byte secteur = 1; secteur <= 3; secteur++) {
    byte premierBloc = secteur * 4;
    byte statut = 1;
    for (byte c = 0; c < 3 && statut != MFRC522::STATUS_OK; c++) {
      if (c > 0) {
        // Un essai raté referme la carte : la réveiller et la resélectionner.
        byte reponseATQA[2];
        byte tailleATQA = sizeof(reponseATQA);
        if (rfid.PICC_WakeupA(reponseATQA, &tailleATQA) != MFRC522::STATUS_OK
            || rfid.PICC_Select(&(rfid.uid), rfid.uid.size * 8) != MFRC522::STATUS_OK)
          break;                        // la carte ne répond plus du tout
      }
      statut = rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, premierBloc,
                                     lesCles[c], &(rfid.uid));
    }
    if (statut != MFRC522::STATUS_OK)
      break;                            // le tiroir résiste à toutes les clés

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
// un repère de début, une longueur, le langage (« en » ou « fr »), le texte.
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

    byte typeLequel = entete & 0x07;            // 1 = contenu « texte »
    const byte* contenu = message + pos + tailleType + tailleId;

    if (typeLequel == 1 && tailleType >= 1 && message[pos] == 'T') {
      byte langueLen = contenu[0] & 0x3F;       // ex : 2 pour « en »
      const byte* mot   = contenu + 1 + langueLen;   // LE mot, enfin !
      int  motLen       = tailleTxt - 1 - langueLen;
      if (motLen > sortieTaille - 1) motLen = sortieTaille - 1;
      if (motLen < 1) break;               // rien de lisible : enveloppe corrompue
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
//  JOUER LE TEXTE (la moitié « protocole » de la machine — c'est ICI que
//  vivent tes décisions !)
// ---------------------------------------------------------------------

bool estMorceauDeMot(char c) {
  return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
    //  || c == '+'                // si ton protocole a des symboles
    //  || (c >= '0' && c <= '9'); // si ton protocole a des chiffres
}

char enMajuscule(char c) {
  return (c >= 'a' && c <= 'z') ? (char)(c - 'a' + 'A') : c;
}

// Découpe le texte en mots, traduit chaque mot avec les tableaux, et joue.
// La DURÉE d'une note : ce qui est écrit derrière le mot — un nombre, ou
// rien du tout (c'est 0 qui arrive alors) — est transmis d'un bloc à la
// fonction dureeEnDemiTemps() : c'est LÀ, et nulle part ailleurs, que ta
// convention de durées vit. (Combien de demi-temps pour chaque écriture ?
// TOI seul le décides — mais la carte et le code doivent dire la même chose !)
void jouerLeTexte(const char* texte) {
  int i = 0;
  bool quelqueChoseAJoue = false;
  while (texte[i] != '\0') {

    // sauter tout ce qui n'est pas un morceau de mot (espaces, virgules...)
    // — sauf un saut de ligne : lui vaut une petite pause d'un temps, la
    // respiration entre deux phrases
    bool respiration = false;
    while (texte[i] != '\0' && ! estMorceauDeMot(texte[i])) {
      if (texte[i] == '\n' || texte[i] == '\r') respiration = true;
      i++;
    }
    if (texte[i] == '\0') break;
    if (respiration) delay(TEMPO_MS);   // le souffle entre les phrases

    int debut = i;
    while (estMorceauDeMot(texte[i])) i++;
    int fin = i;                  // l'index juste après le mot entier
    int longueur = fin - debut;

    // lire ET enlever les chiffres écrits à la fin du mot (ils partent à la
    // convention de durées), le mot du protocole restant seul — sans reculer
    int nombre = 0;
    int valeur = 1;               // pour lire les chiffres de droite à gauche
    while (longueur > 0) {
      char chiffre = texte[debut + longueur - 1];
      if (chiffre < '0' || chiffre > '9') break;
      nombre += (chiffre - '0') * valeur;
      valeur *= 10;
      longueur--;
    }
    i = fin;                      // repartir APRÈS le mot entier : une carte
                                  // qui ne porte que des chiffres ne bloque plus

    if (longueur > 15 || longueur < 1) continue;    // ni trop long, ni vide

    char mot[16];
    for (int k = 0; k < longueur; k++) mot[k] = enMajuscule(texte[debut + k]);
    mot[longueur] = '\0';

    // chercher le mot dans le protocole
    int note = -1;
    for (int n = 0; n < NOTE_COUNT; n++)
      if (strcmp(mot, NOTE_COMMANDS[n]) == 0) note = n;

    if (note >= 0 && NOTE_FREQS[note] > 0) {
      int demisTemps = dureeEnDemiTemps(nombre);
      Serial.print(F("Je joue "));
      Serial.print(mot);
      Serial.print(F(" pendant "));
      if (demisTemps % 2 == 0) Serial.print(demisTemps / 2);
      else { Serial.print(demisTemps); Serial.print(F(" demi-")); }
      Serial.println(F(" temps"));
      jouerUneNote(note, TEMPO_MS * demisTemps / 2);
      quelqueChoseAJoue = true;

    } else if (note >= 0) {             // le mot existe mais la case est vide
      Serial.print(F("Le mot "));
      Serial.print(mot);
      Serial.println(F(" est dans le protocole, mais sa frequence n'est pas choisie !"));
      bipTriste();

    } else if (longueur >= 2) {         // un vrai mot que le protocole ignore
      Serial.print(F("Je ne connais pas \""));
      Serial.print(mot);
      Serial.println(F("\"... le protocole est-il casse ?"));
      bipTriste();
    }
    // (une lettre toute seule, sans chiffre, reste silencieuse)
  }
  if ( ! quelqueChoseAJoue) Serial.println(F("Rien de jouable sur cette carte."));
}

// ---------------------------------------------------------------------
//  EXERCICE 2 — TA CONVENTION DE DURÉE
// ---------------------------------------------------------------------
//
// La musique connaît quatre durées de note : la croche (un
// demi-temps), la noire (un temps), la pointée (un temps et demi)
// et la blanche (deux temps).
//
// Pour TES cartes, c'est à TOI de décider comment chacune s'écrit,
// des DEUX côtés : sur la carte (l'écriture) et ICI (la réponse).
//
// Ce que la machine donne à ta fonction :
//   0            si le mot ne portait RIEN derrière lui,
//   un nombre    si la carte portait un nombre derrière le mot.
//
// Et la fonction doit RÉPONDRE en demi-temps — parce que la croche vaut
// exactement un demi-temps : impossible à dire en temps entiers !
//
int dureeEnDemiTemps(int nombre) {
  return 0;   // ← A TOI : combien de demi-temps pour chaque écriture ?
}

// ---------------------------------------------------------------------
//  FAIRE DU SON (le bout « fréquence → vibration »)
// ---------------------------------------------------------------------

void jouerUneNote(int note, int dureeMs) {
  digitalWrite(LED_PIN, HIGH);              // la lumière, en même temps
  tone(BUZZER_PIN, NOTE_FREQS[note]);       // le buzzer vibre à la fréquence
  delay(dureeMs);
  noTone(BUZZER_PIN);                       // le silence
  digitalWrite(LED_PIN, LOW);
  delay(80);                                // un petit souffle entre les notes
}

void bipTriste() {                          // le son du « protocole cassé »
  tone(BUZZER_PIN, 150); delay(150);
  tone(BUZZER_PIN, 100); delay(200);
  noTone(BUZZER_PIN);
}
