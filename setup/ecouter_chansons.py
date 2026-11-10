# -*- coding: utf-8 -*-
"""
ecouter_chansons.py — l'interpreteur Python du protocole des cartes.

Il applique EXACTEMENT la logique du sketch 02_MusiqueCartes_Nano :
  - couper le texte en mots (lettres, +, chiffres),
  - lire le chiffre de la fin (la durée, comptée en croches),
  - chercher le mot dans le tableau du protocole,
  - jouer la fréquence pendant la durée prévue.

Son rôle : avec l'ordinateur du bureau, ÉCOUTER les chansons du manuel
(classiques + defis 2026) AVANT le jour de l'atelier — et verifier que ce
que les enfants vont ecrire sur leurs cartes sonne comme la vraie chanson.

Trois gammes sont disponibles (le meme protocole, avec ou sans les +) :
  - la gamme double  : DO..SI, DO+..SI+ puis DO++..SI++ (la machine la plus haute),
  - la gamme simple  : DO..SI et DO+..SI+ (sans le ++),
  - la gamme de base : DO..SI et VACHE (7 notes blanches + la noire FA#).
Sur la gamme de base, chaque + descend d'une octave : DO+ sonne comme DO,
RE++ comme RE (deux octaves plus bas), FA#+ comme FA# (et non comme FA).

Utilisation :
  python ecouter_chansons.py                     -> joue TOUTES les chansons du manuel
  python ecouter_chansons.py --sans-plus         -> idem, sur la gamme de base
  python ecouter_chansons.py "DO RE SOL4"        -> joue TON texte de carte
  python ecouter_chansons.py --sans-plus "DO+"   -> ton texte, gamme de base
"""
import sys
import time

try:
    import winsound   # le bip de Windows : onde carree, comme le buzzer
except ImportError:
    sys.exit("Ce programme joue avec winsound : il tourne sur Windows.")

TEMPO_MS = 500          # la duree d'UN temps (une noire), par defaut

# Le protocole : mot -> frequence (les propositions de la machine).
# C'est la gamme COMPLETE : trois octaves, DO a SI, DO+ a SI+, DO++ a SI++,
# et la note noire FA# dans chacune (FA#, FA#+, FA#++).
# (La gamme de base est la premiere ligne, sans les notes + ni ++.)
NOTE_COMMANDS = {
    "DO": 262, "RE": 294, "MI": 330, "FA": 349, "FA#": 370,
    "SOL": 392, "LA": 440, "SI": 494,
    "DO+": 523, "RE+": 588, "MI+": 659, "FA+": 698, "FA#+": 740,
    "SOL+": 784, "LA+": 880, "SI+": 988,
    "DO++": 1046, "RE++": 1175, "MI++": 1318, "FA++": 1397, "FA#++": 1480,
    "SOL++": 1568, "LA++": 1760, "SI++": 1975,
    "VACHE": 200,
}

def duree_en_demi_temps(nombre):
    """La convention de duree - la MEME que dans la fonction du sketch."""
    if nombre == 0: return 2   # sans chiffre : une noire (2 croches)
    return nombre              # le chiffre EST la duree, en croches

def est_morceau_de_mot(c):
    return (c >= 'A' and c <= 'Z') or (c >= 'a' and c <= 'z') \
        or c == '+' or c == '#' or ('0' <= c <= '9')

def en_majuscule(c):
    return c.upper()

def jouer(texte, tempo_ms, avec_plus=True):
    """La meme chose que jouerLeTexte() dans le sketch.

    avec_plus=False : la gamme de base. Les notes + et ++ n'existent pas
    sur cette machine : chaque + descendant d'une octave, on les enlève
    (DO+ devient DO, RE++ devient RE).
    """
    quelquechose = False
    i = 0
    while i < len(texte):
        while i < len(texte) and not est_morceau_de_mot(texte[i]):
            i += 1
        if i == len(texte):
            break
        debut = i
        while i < len(texte) and est_morceau_de_mot(texte[i]):
            i += 1
        mot = texte[debut:i]

        nombre = 0
        valeur = 1
        while mot and mot[-1] >= '0' and mot[-1] <= '9':
            nombre += int(mot[-1]) * valeur   # les chiffres, de droite a gauche
            valeur *= 10
            mot = mot[:-1]
        mot = mot.upper()

        if not avec_plus and mot.endswith('+'):
            sans = mot.replace('+', '')
            if len(sans) >= 1:
                if len(mot) - len(sans) == 1:
                    print("  (gamme de base :", mot, "->", sans, ", une octave plus bas)")
                else:
                    print("  (gamme de base :", mot, "->", sans, ", deux octaves plus bas)")
                mot = sans

        freq = NOTE_COMMANDS.get(mot)
        if freq and freq > 0:
            demis = duree_en_demi_temps(nombre)
            duree = tempo_ms * demis // 2
            if demis % 2 == 0:
                print("  Je joue", mot, "pendant", demis // 2, "temps")
            else:
                print("  Je joue", mot, "pendant", demis, "demi-temps")
            winsound.Beep(freq, duree)
            time.sleep(0.08)           # le petit souffle entre les notes
            quelquechose = True
        elif freq is not None:
            print("  Le mot", mot, "est dans le protocole, mais sa frequence n'est pas choisie !")
        elif len(mot) >= 2:
            print("  Je ne connais pas \"", mot, "\"... le protocole est-il casse ?")
            # (dans la vraie machine, un bip triste remplace ce message)
    if not quelquechose:
        print("  Rien de jouable sur cette carte.")

# Les chansons, ECRITES dans l'encodage de DEMONSTRATION de la machine
# complete (03_MusiqueCartes_Nano_COMPLETE) : c'est la correction de
# reference. Pendant l'atelier, chaque enfant les recrit dans SON propre
# protocole ! Chaque ligne = une phrase, elle-meme une suite de cartes.
# Chaque entree = (nom, TEMPO_MS, lignes de carte) — chacune recopiée
# VER BATIM du tableau correspondant du manuel (l'enfant la copie telle quelle).
CHANSONS_CLASSIQUES = [
    ("Au clair de la lune", 500, [
        "DO DO DO RE MI4 RE DO MI4 RE RE DO4",
    ]),
    ("Frere Jacques", 450, [
        "DO RE MI DO DO RE MI DO MI FA SOL4 MI FA SOL4 SOL LA SOL FA MI DO SOL LA SOL FA MI DO DO SOL DO DO SOL DO",
    ]),
    ("Ah ! vous dirai-je, maman (Twinkle)", 500, [
        "DO DO SOL SOL LA LA SOL4 FA FA MI MI RE RE DO4",
    ]),
    ("Ode a la joie (version simplifiee)", 450, [
        "MI MI FA SOL SOL FA MI RE DO DO RE MI MI3 RE1 RE4",
    ]),
    ("Vive le vent", 430, [
        "MI MI MI4 MI MI MI4 MI SOL DO RE MI4",
    ]),
    ("Joyeux anniversaire", 460, [
        "SOL SOL LA SOL DO+ SI4",
        "SOL SOL LA SOL RE+ DO+4",
        "SOL SOL SOL MI DO+ SI4 LA4",
        "FA FA MI DO+ RE+ DO+4"
    ]),
    ("Une souris verte (le debut)", 450, [
        "SOL1 SOL1 SOL1 LA1 SOL RE",
    ]),
]

CHANSONS_2026 = [
    ("Golden - HUNTR/X (la pre-chanson puis tout le refrain)", 380, [
        "LA DO FA MI MI4",
        "SOL SOL LA SOL SOL4",
        "SI RE SOL FA FA MI4",
        "MI MI LA LA LA LA",
        "MI MI SOL SOL RE",
        "SOL SOL SOL SOL FA FA FA MI",
        "MI MI LA LA LA LA",
        "MI MI SOL SOL RE",
        "SOL SOL SOL SOL FA FA FA MI",
        "LA SI SOL",
        "SI LA SOL RE"
    ]),
    ("APT. - Rose & Bruno Mars (le hook : une seule note, le rythme fait tout)", 280, [
        "DO4 DO1 DO1 DO1 DO4 DO4 DO1 DO1",
        "DO4 DO1 DO1 DO1 DO4 DO4 DO1 DO1",
        "DO1 DO1 DO1 DO1 DO DO DO DO DO4",
        "DO4 DO1 DO1 DO1 DO4"
    ])
]

def jouer_chanson(nom, tempo, lignes, avec_plus=True):
    print()
    print("=== ", nom, "   (tempo", tempo, "ms) ===")
    time.sleep(0.4)
    for ligne in lignes:
        print("[cartes :", ligne, "]")
        jouer(ligne, tempo, avec_plus)
        time.sleep(0.3)             # une respiration entre deux phrases

if __name__ == "__main__":
    avec_plus = True
    mots = []
    for argument in sys.argv[1:]:
        if argument == "--sans-plus":
            avec_plus = False
        else:
            mots.append(argument)

    if avec_plus:
        print("Gamme : complete (avec les +)")
    else:
        print("Gamme : de base (sans les +)")

    if mots:
        print("[TA CARTE : \"", " ".join(mots), "\"]")
        jouer(" ".join(mots), TEMPO_MS, avec_plus)
    else:
        print("Les classiques :")
        for nom, tempo, lignes in CHANSONS_CLASSIQUES:
            jouer_chanson(nom, tempo, lignes, avec_plus)
        print()
        print("Les defis 2026 :")
        for nom, tempo, lignes in CHANSONS_2026:
            print(nom)
            jouer_chanson(nom, tempo, lignes, avec_plus)
        print()
        print("Fin ! Si une chanson sonne faux, c'est son ecriture de cartes")
        print("qu'il faut corriger dans le manuel - pas la machine.")
