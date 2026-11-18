# 17 — HAL

## Énoncé / données
`rahaha_le_code_est_la.txt` : 50 000 caractères en minuscules sur une seule ligne, sans retour à la ligne. Un code est caché dedans.

## Solveur
`halcode.c` charge le texte avec `charger_texte` (code partagé à la racine : `charge_fichier.c` / `fichier.h`), puis doit en extraire le code via `trouver_code`.

## Compiler / lancer
Depuis ce dossier :

```
make
make run
```

## État
En cours. Le code partagé ne compile pas encore (voir la section « Known state » du `CLAUDE.md` à la racine). Aucune stratégie de résolution n'est encore implémentée ; `main` se contente de charger le fichier.
