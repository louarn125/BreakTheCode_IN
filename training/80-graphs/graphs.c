/*
 * Épreuve 80 — aller de Xevora à Crystar en payant chaque trajet avec du
 * carburant ramassé en chemin, en maximisant la valeur des armes à l'arrivée.
 *
 * Recherche en faisceau (beam search) sur les états (planète, inventaire).
 * Usage : ./graphs <capacité de poids> [largeur du faisceau]
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PLANETES 256
#define MAX_ARETES   1024
#define MAX_ITEMS    1024
#define MAX_PAS      16
#define NB_MOTS      (MAX_ITEMS / 64)
#define DEPART       "Xevora"
#define ARRIVEE      "Crystar"

typedef struct { int dep, arr, cout; } Arete;
typedef struct { int id, planete, valeur, poids, arme; } Item;

static char noms[MAX_PLANETES][64];
static int nb_planetes;
static Arete aretes[MAX_ARETES];
static int nb_aretes;
static Item items[MAX_ITEMS];
static int nb_items;
static int capacite;

/* Un pas du trajet : arête empruntée, carburants brûlés, objets ramassés. */
typedef struct {
    int parent;               /* indice dans le niveau précédent, -1 au départ */
    int planete;
    int brules[32], nb_brules;   /* indices dans items, -1 = carburant 1 */
    int ramasses[64], nb_ramasses;
} Pas;

typedef struct {
    int planete, poids, armes, fuel;
    unsigned long long inv[NB_MOTS];   /* objets possédés */
    unsigned long long vus[NB_MOTS];   /* objets déjà pris (ou brûlés) */
    int pas;                           /* indice dans niveaux[niv] */
    double score;
} Etat;

static int planete_id(const char *nom)
{
    for (int i = 0; i < nb_planetes; i++)
        if (strcmp(noms[i], nom) == 0) return i;
    strcpy(noms[nb_planetes], nom);
    return nb_planetes++;
}

static int charger(void)
{
    char ligne[256];
    FILE *f = fopen("Edges", "r");
    if (!f) { perror("Edges"); return -1; }
    if (!fgets(ligne, sizeof ligne, f)) return -1;
    while (fgets(ligne, sizeof ligne, f)) {
        char a[64], b[64]; int c;
        if (sscanf(ligne, "%63[^,],%63[^,],%d", a, b, &c) != 3) continue;
        aretes[nb_aretes++] = (Arete){ planete_id(a), planete_id(b), c };
    }
    fclose(f);

    f = fopen("Items", "r");
    if (!f) { perror("Items"); return -1; }
    if (!fgets(ligne, sizeof ligne, f)) return -1;
    while (fgets(ligne, sizeof ligne, f)) {
        char p[64], t[16]; int id, v, w;
        if (sscanf(ligne, "%d,%63[^,],%d,%d,%15s", &id, p, &v, &w, t) != 5) continue;
        items[nb_items++] = (Item){ id, planete_id(p), v, w, strcmp(t, "WEAPON") == 0 };
    }
    fclose(f);
    return 0;
}

static int possede(const Etat *e, int i) { return (e->inv[i / 64] >> (i % 64)) & 1; }

/* Choisit les carburants à brûler pour payer `cout` : somme minimale >= cout
 * (puis poids libéré maximal), par programmation dynamique. Renvoie le nombre
 * de carburants, ou -1 si impossible. */
static int payer(const Etat *e, int cout, int *sel)
{
    int fu[MAX_ITEMS], nf = 0, vmax = 0;
    for (int i = 0; i < nb_items; i++)
        if (!items[i].arme && possede(e, i)) {
            fu[nf++] = i;
            if (items[i].valeur > vmax) vmax = items[i].valeur;
        }
    int smax = cout + vmax;
    int *poids = malloc((smax + 1) * sizeof(int));
    int *pris = malloc((size_t)(nf + 1) * (smax + 1) * sizeof(int));
    /* poids[s] = poids max libéré pour une somme exacte s, -1 si inatteignable */
    for (int s = 0; s <= smax; s++) poids[s] = -1;
    poids[0] = 0;
    memset(pris, 0, (size_t)(nf + 1) * (smax + 1) * sizeof(int));
    for (int k = 0; k < nf; k++) {
        int v = items[fu[k]].valeur, w = items[fu[k]].poids;
        int *cour = pris + (size_t)(k + 1) * (smax + 1);
        int *nouv = malloc((smax + 1) * sizeof(int));
        memcpy(nouv, poids, (smax + 1) * sizeof(int));
        for (int s = 0; s + v <= smax; s++)
            if (poids[s] >= 0 && poids[s] + w > nouv[s + v]) {
                nouv[s + v] = poids[s] + w;
                cour[s + v] = 1;
            }
        memcpy(poids, nouv, (smax + 1) * sizeof(int));
        free(nouv);
    }
    int best = -1;
    for (int s = cout; s <= smax; s++)
        if (poids[s] >= 0) { best = s; break; }
    int n = -1;
    if (best >= 0) {
        n = 0;
        int s = best;
        for (int k = nf; k >= 1; k--) {
            if (pris[(size_t)k * (smax + 1) + s]) {
                sel[n++] = fu[k - 1];
                s -= items[fu[k - 1]].valeur;
            }
        }
    }
    free(poids); free(pris);
    return n;
}

static int cmp_items(const void *a, const void *b)
{
    const Item *x = &items[*(const int *)a], *y = &items[*(const int *)b];
    /* valeur/poids décroissant */
    long l = (long)x->valeur * y->poids, r = (long)y->valeur * x->poids;
    return (l < r) - (l > r);
}

static const int FRACS[] = { 0, 25, 50, 75, 100 };
#define NB_FRACS 5

static int cmp_score(const void *a, const void *b)
{
    double x = ((const Etat *)a)->score, y = ((const Etat *)b)->score;
    return (x < y) - (x > y);
}

static double evaluer(const Etat *e)
{
    return e->armes + 0.6 * e->fuel - 0.05 * e->poids;
}

int main(int argc, char **argv)
{
    if (argc < 2) { fprintf(stderr, "usage: %s <capacité> [faisceau]\n", argv[0]); return 1; }
    capacite = atoi(argv[1]);
    int largeur = argc > 2 ? atoi(argv[2]) : 3000;
    if (charger() != 0) return 1;

    int dep = planete_id(DEPART), arr = planete_id(ARRIVEE);

    /* items triés par ratio, par planète et par type */
    int *tri = malloc(sizeof(int) * nb_items);
    for (int i = 0; i < nb_items; i++) tri[i] = i;
    qsort(tri, nb_items, sizeof(int), cmp_items);

    Pas *niveaux[MAX_PAS + 1];
    int taille_niv[MAX_PAS + 1];
    size_t cap_pas = (size_t)largeur * 64;
    for (int i = 0; i <= MAX_PAS; i++) niveaux[i] = malloc(cap_pas * sizeof(Pas));

    Etat *cour = malloc(sizeof(Etat) * largeur);
    size_t cap_next = (size_t)largeur * 400;
    Etat *suiv = malloc(sizeof(Etat) * cap_next);
    int ncour = 1;
    memset(&cour[0], 0, sizeof(Etat));
    cour[0].planete = dep;
    cour[0].pas = 0;
    niveaux[0][0] = (Pas){ .parent = -1, .planete = dep };
    taille_niv[0] = 1;
    taille_niv[1] = 0;

    double meilleur = -1;
    int meilleur_niv = -1, meilleur_pas = -1;

    for (int niv = 1; niv <= MAX_PAS && ncour > 0; niv++) {
        size_t nsuiv = 0;
        for (int c = 0; c < ncour; c++) {
            Etat *e = &cour[c];
            for (int a = 0; a < nb_aretes; a++) {
                if (aretes[a].dep != e->planete) continue;
                int sel[64], n;
                Etat base = *e;
                int premier = (niv == 1);
                if (premier) n = 0;
                else if ((n = payer(e, aretes[a].cout, sel)) < 0) continue;
                for (int k = 0; k < n; k++) {
                    int i = sel[k];
                    base.inv[i / 64] &= ~(1ULL << (i % 64));
                    base.poids -= items[i].poids;
                    base.fuel -= items[i].valeur;
                }
                base.planete = aretes[a].arr;

                /* variantes de ramassage : fraction de capacité libre pour
                 * les armes, puis pour les carburants */
                for (int fa = 0; fa < NB_FRACS; fa++)
                for (int ff = 0; ff < NB_FRACS; ff++) {
                    if (base.planete == arr && ff > 0) continue; /* carburant inutile */
                    if (nsuiv >= cap_next) break;
                    Etat s = base;
                    int ram[64], nr = 0;
                    int libre = capacite - s.poids;
                    for (int pass = 0; pass < 2; pass++) {
                        /* pass 0 : armes, pass 1 : carburants */
                        int bud = pass == 0 ? libre * FRACS[fa] / 100
                                           : (capacite - s.poids) * FRACS[ff] / 100;
                        for (int t = 0; t < nb_items; t++) {
                            int i = tri[t];
                            if (items[i].planete != base.planete) continue;
                            if (items[i].arme != (pass == 0)) continue;
                            if ((s.vus[i / 64] >> (i % 64)) & 1) continue;
                            if (items[i].poids > bud || nr >= 64) continue;
                            bud -= items[i].poids;
                            s.inv[i / 64] |= 1ULL << (i % 64);
                            s.vus[i / 64] |= 1ULL << (i % 64);
                            s.poids += items[i].poids;
                            if (items[i].arme) s.armes += items[i].valeur;
                            else s.fuel += items[i].valeur;
                            ram[nr++] = i;
                        }
                    }
                    s.score = evaluer(&s);
                    /* doublon exact (même planète, même inventaire) */
                    int doublon = 0;
                    for (size_t q = nsuiv; q-- > 0 && q + 40 > nsuiv;)
                        if (suiv[q].planete == s.planete &&
                            memcmp(suiv[q].inv, s.inv, sizeof s.inv) == 0 &&
                            memcmp(suiv[q].vus, s.vus, sizeof s.vus) == 0) { doublon = 1; break; }
                    if (doublon) continue;

                    int idx = taille_niv[niv];
                    if (idx >= (int)cap_pas) continue;
                    Pas *p = &niveaux[niv][idx];
                    p->parent = e->pas;
                    p->planete = s.planete;
                    p->nb_brules = premier ? 1 : n;
                    if (premier) p->brules[0] = -1;
                    else memcpy(p->brules, sel, n * sizeof(int));
                    p->nb_ramasses = nr;
                    memcpy(p->ramasses, ram, nr * sizeof(int));
                    taille_niv[niv]++;
                    s.pas = idx;
                    suiv[nsuiv++] = s;
                }
            }
        }
        /* arrivées à Crystar : candidats finaux, on n'étend pas plus loin */
        size_t m = 0;
        for (size_t q = 0; q < nsuiv; q++) {
            if (suiv[q].planete == arr) {
                if (suiv[q].armes > meilleur) {
                    meilleur = suiv[q].armes;
                    meilleur_niv = niv;
                    meilleur_pas = suiv[q].pas;
                }
            } else suiv[m++] = suiv[q];
        }
        qsort(suiv, m, sizeof(Etat), cmp_score);
        ncour = m < (size_t)largeur ? (int)m : largeur;
        memcpy(cour, suiv, ncour * sizeof(Etat));
        if (niv < MAX_PAS) taille_niv[niv + 1] = 0;
    }

    if (meilleur_niv < 0) { fprintf(stderr, "aucune solution trouvée\n"); return 1; }

    /* reconstitution du chemin */
    int chemin[MAX_PAS + 1], n = 0;
    for (int niv = meilleur_niv, p = meilleur_pas; niv >= 1; niv--) {
        chemin[n++] = p;
        p = niveaux[niv][p].parent;
    }
    int niv = 1;
    for (int k = n - 1; k >= 0; k--, niv++) {
        Pas *p = &niveaux[niv][chemin[k]];
        printf("%s,", noms[p->planete]);
        for (int j = 0; j < p->nb_brules; j++)
            printf(j ? ";%d" : "%d", p->brules[j] < 0 ? 1 : items[p->brules[j]].id);
        printf(",");
        if (p->nb_ramasses == 0) printf("N/A");
        for (int j = 0; j < p->nb_ramasses; j++)
            printf(j ? ";%d" : "%d", items[p->ramasses[j]].id);
        printf("\n");
    }
    fprintf(stderr, "score = %.0f (capacité %d, faisceau %d)\n", meilleur, capacite, largeur);
    return 0;
}
