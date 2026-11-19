/*
 * template.c : squelette de solveur pour les épreuves Break the Code.
 *
 * Lit un fichier d'entrée en entier, appelle resoudre(), écrit le résultat
 * dans un fichier de sortie.
 *
 * Usage : ./template [entree] [sortie]
 *   entree : fichier à lire      (défaut : ENTREE_DEFAUT)
 *   sortie : fichier à écrire    (défaut : SORTIE_DEFAUT)
 *
 * Pour une nouvelle épreuve : copier ce fichier dans training/<NN>-<nom>/,
 * adapter ENTREE_DEFAUT et remplir resoudre().
 */

#include <stdio.h>
#include <stdlib.h>

#define ENTREE_DEFAUT "entree.txt"
#define SORTIE_DEFAUT "output.txt"

/* Lit tout le fichier dans un buffer alloué et terminé par '\0'.
 * Écrit la taille lue dans *taille (peut être NULL).
 * Renvoie NULL en cas d'erreur ; l'appelant doit faire free(). */
static char *lire_fichier(const char *chemin, size_t *taille)
{
    FILE *f = fopen(chemin, "rb");
    if (f == NULL) {
        perror(chemin);
        return NULL;
    }

    size_t capacite = 4096;
    size_t n = 0;
    char *buf = malloc(capacite);
    if (buf == NULL) {
        fclose(f);
        return NULL;
    }

    size_t lu;
    while ((lu = fread(buf + n, 1, capacite - n - 1, f)) > 0) {
        n += lu;
        if (n + 1 >= capacite) {
            capacite *= 2;
            char *tmp = realloc(buf, capacite);
            if (tmp == NULL) {
                free(buf);
                fclose(f);
                return NULL;
            }
            buf = tmp;
        }
    }

    if (ferror(f)) {
        perror(chemin);
        free(buf);
        fclose(f);
        return NULL;
    }

    fclose(f);
    buf[n] = '\0';
    if (taille != NULL)
        *taille = n;
    return buf;
}

/* Écrit les `taille` octets de `donnees` dans `chemin` (écrase le fichier).
 * Renvoie 0 si OK, -1 sinon. */
static int ecrire_fichier(const char *chemin, const char *donnees, size_t taille)
{
    FILE *f = fopen(chemin, "wb");
    if (f == NULL) {
        perror(chemin);
        return -1;
    }

    if (fwrite(donnees, 1, taille, f) != taille) {
        perror(chemin);
        fclose(f);
        return -1;
    }

    if (fclose(f) != 0) {
        perror(chemin);
        return -1;
    }
    return 0;
}

/* À remplir : traite `entree` (taille `n`, terminée par '\0') et renvoie
 * le résultat dans un buffer alloué terminé par '\0' (free() par l'appelant),
 * ou NULL en cas d'erreur. */
static char *resoudre(const char *entree, size_t n)
{
    (void)n;

    /* TODO : remplacer par la vraie résolution. Ici : recopie de l'entrée. */
    size_t len = 0;
    while (entree[len] != '\0')
        len++;

    char *sortie = malloc(len + 1);
    if (sortie == NULL)
        return NULL;
    for (size_t i = 0; i <= len; i++)
        sortie[i] = entree[i];
    return sortie;
}

int main(int argc, char *argv[])
{
    const char *chemin_entree = argc > 1 ? argv[1] : ENTREE_DEFAUT;
    const char *chemin_sortie = argc > 2 ? argv[2] : SORTIE_DEFAUT;

    size_t n = 0;
    char *entree = lire_fichier(chemin_entree, &n);
    if (entree == NULL)
        return EXIT_FAILURE;

    char *sortie = resoudre(entree, n);
    free(entree);
    if (sortie == NULL) {
        fprintf(stderr, "resoudre a échoué\n");
        return EXIT_FAILURE;
    }

    size_t taille_sortie = 0;
    while (sortie[taille_sortie] != '\0')
        taille_sortie++;

    int ret = ecrire_fichier(chemin_sortie, sortie, taille_sortie);
    free(sortie);
    if (ret != 0)
        return EXIT_FAILURE;

    printf("Résultat écrit dans %s\n", chemin_sortie);
    return EXIT_SUCCESS;
}
