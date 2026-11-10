# src/ — qui touche quoi ?

Deux mondes ne se mélangent pas.

## animateur/ — le monde de l'animateur. Les enfants n'y ouvrent JAMAIS rien.

- `01_Test_Nano` — le test de câblage (buzzer + LED, sans lecteur).
  Flashé par un adulte AVANT l'atelier ; pendant l'atelier personne
  ne l'édite.
- `03_MusiqueCartes_Nano_COMPLETE` — **LE code de travail officiel** :
  la machine complète (le corrigé, tous les exercices résolus),
  flashée sur la machine de démonstration. Quand un réglage est
  changé pour de vrai, il change ICI d'abord, puis il est reporté
  à l'identique du côté enfant (des DEUX côtés, un seul protocole !)
- Et pour **copier la correction vers un fichier enfant** : les deux
  fichiers sont identiques au mot près — les trois seuls morceaux à
  reporter sont 1) la ligne `NOTE_COMMANDS` (2 lignes), 2) la ligne
  `NOTE_FREQS` (2 lignes), 3) le corps de la fonction
  `dureeEnDemiTemps`. Tout le reste est déjà identique.
- `DEFI_4_fanfare.txt` — le mode d'emploi du Défi 4 (la porte secrète) :
  à copier-coller dans le fichier d'un groupe, avec les élèves.

## enfants/ — le monde des groupes. Chaque kit édite SON exemplaire.

- `02_MusiqueCartes_Nano` — **LE fichier que les enfants ouvrent et
  éditent** pendant l'atelier : les tableaux du protocole laissés VIDES
  (à eux d'inventer mots et fréquences !), et trois zones marquées
  EXERCICE 1 / 2 / 3 (les mots du protocole, la durée, le tempo).
  Sa logique est identique, au mot près, à la version de l'animateur —
  sa seule différence doit être les trois exercices non résolus.

## planB_ESP32/ — la même machine sur carte ESP32 (si les kits arrivent).

Mêmes rôles qu'en haut : `01_Test` (animateur), `02_MusiqueCartes`
(enfants), `03_MusiqueCartes_COMPLETE` (animateur). `_anciens/` =
vieux brouillons, référence seulement, jamais ouverts.

## Et en dehors de src/ (tout côté animateur)

- `setup/SESSION_PLAN.md` — le déroulé de l'atelier et les décisions.
- `setup/ARDUINO-INIT.md` — l'installation de l'IDE (une seule fois,
  puis les quatre réglages du menu Tools avant CHAQUE téléversement).
- `setup/ecouter_chansons.py` — écouter à l'avance les chansons du
  manuel avec la logique de la machine (`python ecouter_chansons.py`
  depuis le dossier setup ; pour un essai sans bip audible, voir AGENTS.md).
- `kid resources/manual/manuel_enfant.html` — LE livret source des
  enfants (le PDF se régénère après chaque modification).
