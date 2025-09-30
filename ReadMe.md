# Systèmes embarqués : implémentation sur micro:bit

## Cadre 

Ce projet a été réalisé en binôme avec Corentin CLERC en 1ère année de master de sécurité informatique à l'Université de Limoges, dans le cadre de l'UE Systèmes embarqués encadrée par Mr Pierre-François Bonnefoi.

## Description

L'objectif de ce projet était de réaliser deux firmware sur micro:bit, le premier permettant à chaque micro:bit de sélectionner et d’échanger des emojis via radio, et le deuxième
permettant à un utilisateur de dessiner sur un micro:bit et d’envoyer le dessin à l'autre via radio. 

## Fonctionnement

### `firmware1.c`

- Affichage : une tâche dédiée affiche l’emoji correspondant à un compteur global.
- Choix de l’emoji : avec les boutons A et B, l’utilisateur incrémente ou décrémente le compteur pour parcourir les emojis disponibles.
- Réception du message : le micro:bit reçoit un compteur envoyé par radio, déclenche un beep, puis met à jour l’emoji affiché selon le compteur reçu.
- Gestion des priorités : ordre des tâches défini de manière à éviter les collisions et donner la priorité à l’action locale de l’utilisateur.

### `firmware2.c`
- Gestion de l’image : une matrice 5x5 représente le dessin et est ensuite convertie pour être affichée sur le micro:bit.
- Tâche principale : affiche en continu l’image construite à partir de la matrice.
- Curseur : une tâche séparée fait clignoter le curseur pour montrer la position de dessin active (sans ralentir l’affichage, voir `rapport.pdf`).
- Dessin : gestion des entrées (pression courte/longue des boutons) pour modifier les pixels du dessin.
- Envoi : conversion du dessin (matrice 5x5) en tableau de caractères puis envoi par radio.
- Réception : le micro:bit convertit le message reçu en dessin, le copie dans son affichage, et joue un beep.
