# 80 — Graphes

## Énoncé
Voir `instruction.txt` : aller de Xevora à Crystar (arêtes orientées). Chaque trajet
doit être payé par des carburants possédés dont la somme des valeurs est ≥ au coût
(ils sont consommés). Le carburant 1 (valeur infinie, poids 0) paie le premier trajet.
Score = somme des valeurs des armes possédées à l'arrivée. Format de réponse :
`planète,carburants (;),objets ramassés (; ou N/A)`.

**Capacité de poids du vaisseau : non fournie dans l'énoncé** (renvoie à « l'exercice
précédent »). Elle est donc passée en paramètre (`CAP`, défaut 100 : valeur arbitraire).

## Données
CSV sans extension, avec en-tête :

- `Edges` (276 lignes) : `departure,arrival,cost`.
- `Items` (694 lignes) : `id,planete,value,weight,type` (`WEAPON` ou `FUEL`).

Les fichiers `*:Zone.Identifier` sont des résidus de téléchargement Windows/WSL : à ignorer.

## Solveur (`graphs.c`)
Recherche en faisceau sur les états (planète, inventaire), 16 pas max. À chaque arête,
les carburants à brûler sont choisis par programmation dynamique (somme minimale ≥ coût,
poids libéré maximal) ; à l'arrivée, plusieurs politiques de ramassage (fraction de la
capacité libre allouée aux armes puis aux carburants, par ratio valeur/poids). Heuristique :
pas de garantie d'optimalité, augmenter `BEAM` pour chercher mieux.

```
make run CAP=100 BEAM=3000 > solution.csv   # score affiché sur stderr
make clean
```

## État
Solveur fonctionnel, solutions vérifiées avec un validateur indépendant. Scores obtenus
(BEAM=3000) : CAP=50 → 113, 100 → 137, 200 → 181, 1000 → 209. Reste à relancer avec la
vraie capacité.
