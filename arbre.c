#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "arbre.h"
#include "types.h"   // pour accéder à MAX_LEN, NoeudArbre, Racine, Derive

// -----------------------------------------------------------------------------
// Fonctions de base AVL
// -----------------------------------------------------------------------------

NoeudArbre* creerNoeud(const char* racineStr) {
    NoeudArbre* n = (NoeudArbre*)malloc(sizeof(NoeudArbre));
    if (!n) return NULL;

    strncpy(n->data.racine, racineStr, MAX_LEN - 1);
    n->data.racine[MAX_LEN - 1] = '\0';
    n->data.derives = NULL;
    n->gauche = n->droite = NULL;
    n->hauteur = 1;
    return n;
}

int max(int a, int b) {
    return a > b ? a : b;
}

int hauteur(NoeudArbre* n) {
    return n ? n->hauteur : 0;
}

int equilibre(NoeudArbre* n) {
    return n ? hauteur(n->gauche) - hauteur(n->droite) : 0;
}

NoeudArbre* rotationDroite(NoeudArbre* y) {
    NoeudArbre* x = y->gauche;
    NoeudArbre* T2 = x->droite;

    x->droite = y;
    y->gauche = T2;

    y->hauteur = max(hauteur(y->gauche), hauteur(y->droite)) + 1;
    x->hauteur = max(hauteur(x->gauche), hauteur(x->droite)) + 1;

    return x;
}

NoeudArbre* rotationGauche(NoeudArbre* x) {
    NoeudArbre* y = x->droite;
    NoeudArbre* T2 = y->gauche;

    y->gauche = x;
    x->droite = T2;

    x->hauteur = max(hauteur(x->gauche), hauteur(x->droite)) + 1;
    y->hauteur = max(hauteur(y->gauche), hauteur(y->droite)) + 1;

    return y;
}

// -----------------------------------------------------------------------------
// Insertion avec équilibrage AVL
// -----------------------------------------------------------------------------

NoeudArbre* insererRacine(NoeudArbre* racine, const char* nouvelleRacine) {
    if (racine == NULL) {
        return creerNoeud(nouvelleRacine);
    }

    int cmp = strcmp(nouvelleRacine, racine->data.racine);
    if (cmp < 0) {
        racine->gauche = insererRacine(racine->gauche, nouvelleRacine);
    } else if (cmp > 0) {
        racine->droite = insererRacine(racine->droite, nouvelleRacine);
    } else {
        return racine;  // déjà présente, on ne fait rien
    }

    // Mise à jour de la hauteur
    racine->hauteur = 1 + max(hauteur(racine->gauche), hauteur(racine->droite));

    // Calcul du facteur d'équilibre
    int eq = equilibre(racine);

    // Cas Left Left
    if (eq > 1 && strcmp(nouvelleRacine, racine->gauche->data.racine) < 0)
        return rotationDroite(racine);

    // Cas Left Right
    if (eq > 1 && strcmp(nouvelleRacine, racine->gauche->data.racine) > 0) {
        racine->gauche = rotationGauche(racine->gauche);
        return rotationDroite(racine);
    }

    // Cas Right Right
    if (eq < -1 && strcmp(nouvelleRacine, racine->droite->data.racine) > 0)
        return rotationGauche(racine);

    // Cas Right Left
    if (eq < -1 && strcmp(nouvelleRacine, racine->droite->data.racine) < 0) {
        racine->droite = rotationDroite(racine->droite);
        return rotationGauche(racine);
    }

    return racine;
}

// -----------------------------------------------------------------------------
// Recherche
// -----------------------------------------------------------------------------

NoeudArbre* rechercherRacine(NoeudArbre* racine, const char* cle) {
    if (racine == NULL) return NULL;

    int cmp = strcmp(cle, racine->data.racine);
    if (cmp == 0) return racine;
    if (cmp < 0) return rechercherRacine(racine->gauche, cle);
    return rechercherRacine(racine->droite, cle);
}

// -----------------------------------------------------------------------------
// Affichage trié alphabétiquement (collecte + qsort)
// -----------------------------------------------------------------------------

static int comparerRacines(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

static int compterNoeuds(NoeudArbre *n) {
    if (n == NULL) return 0;
    return 1 + compterNoeuds(n->gauche) + compterNoeuds(n->droite);
}

static void remplirTableau(NoeudArbre *n, const char **tab, int *idx) {
    if (n == NULL) return;
    remplirTableau(n->gauche, tab, idx);
    tab[*idx] = n->data.racine;
    (*idx)++;
    remplirTableau(n->droite, tab, idx);
}

void afficherRacines(NoeudArbre *racine) {
    if (racine == NULL) {
        printf("Aucune racine stockée.\n");
        return;
    }

    int count = compterNoeuds(racine);
    if (count == 0) return;

    const char **tab = malloc(count * sizeof(const char *));
    if (tab == NULL) {
        printf("Erreur : allocation mémoire impossible.\n");
        return;
    }

    int idx = 0;
    remplirTableau(racine, tab, &idx);

    // Tri (même si l'ordre est "inversé", on va compenser après)
    qsort(tab, count, sizeof(const char *), comparerRacines);

    // AFFICHAGE INVERSÉ pour obtenir l'ordre arabe naturel (de أ à ي)
    printf("\nRacines stockées (triées alphabétiquement - ordre arabe) :\n");
    printf("----------------------------------------\n");

    for (int i = count - 1; i >= 0; i--) {   // ← On inverse ici !
        printf("  %2d. %s\n", count - i, tab[i]);
    }

    printf("----------------------------------------\n");
    printf("Total : %d racine(s)\n\n", count);

    free(tab);
}
// -----------------------------------------------------------------------------
// Libération mémoire
// -----------------------------------------------------------------------------

void libererDerives(Derive* d) {
    while (d) {
        Derive* tmp = d;
        d = d->suivant;
        free(tmp);
    }
}

void libererArbre(NoeudArbre* racine) {
    if (!racine) return;
    libererArbre(racine->gauche);
    libererArbre(racine->droite);
    libererDerives(racine->data.derives);
    free(racine);
}
// Retourne la racine sélectionnée par numéro (1-based), ou NULL si choix invalide
const char* selectionnerRacineParNumero(NoeudArbre* racine) {
    if (!racine) {
        printf("Aucune racine disponible.\n");
        return NULL;
    }

    int count = compterNoeuds(racine);
    if (count == 0) return NULL;

    const char** tab = malloc(count * sizeof(const char*));
    if (!tab) {
        printf("Erreur allocation mémoire.\n");
        return NULL;
    }

    int idx = 0;
    remplirTableau(racine, tab, &idx);

    qsort(tab, count, sizeof(const char*), comparerRacines);

    printf("\nChoisissez une racine par son numéro :\n");
    printf("───────────────────────────────────────────────\n");

    for (int i = 0; i < count; i++) {
        printf("  %2d. %s\n", i + 1, tab[i]);
    }
    printf("───────────────────────────────────────────────\n");
    printf("Entrez le numéro (1 à %d) : ", count);

    int choix;
    if (scanf("%d", &choix) != 1 || choix < 1 || choix > count) {
        printf("Choix invalide.\n");
        free(tab);
        return NULL;
    }

    const char* selectionnee = tab[choix - 1];
    free(tab);

    return selectionnee;
}