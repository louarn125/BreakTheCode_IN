# 47 — Message

## Énoncé / données
`message.wav` : un message en code Morse encodé comme un son (WAV PCM 16 bits) sur une fréquence porteuse donnée.

## Solveur
`morse.c`, autonome :
1. lit l'en-tête WAV et les échantillons (PCM 16 bits requis) ;
2. découpe le signal en fenêtres de 5 ms et mesure l'énergie à la fréquence cible (algorithme de Goertzel) ;
3. seuille à 30 % du maximum pour obtenir des plages « allumé / éteint » ;
4. convertit les durées en points, traits et espaces, puis en lettres et chiffres (`?` si le symbole est inconnu).

## Compiler / lancer
Depuis ce dossier :

```
make
make run FREQ=700
```

`FREQ` est la fréquence du signal en Hz (700 par défaut). Si le décodage donne des `?`, essayer une autre fréquence.
