#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "arbre.h"

NoeudArbre* creerNoeud(const char* racineStr) {
    NoeudArbre* n = (NoeudArbre*)malloc(sizeof(NoeudArbre));
    if (!n) return NULL;
    strncpy(n->data.racine, racineStr, MAX_LEN - 1);
    n->data.racine[MAX_LEN - 1] = '\0';
    n->data.derives = NULL;
    n->gauche = n->droite = NULL;
    n->hauteur = 1; // pour AVL plus tard, sinon pas grave
    return n;
}

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
        // déjà présente, ne rien faire ou gérer autrement
    }
    // ici tu pourras plus tard mettre à jour la hauteur et faire les rotations AVL
    return racine;
}

NoeudArbre* rechercherRacine(NoeudArbre* racine, const char* cle) {
    if (racine == NULL) return NULL;
    int cmp = strcmp(cle, racine->data.racine);
    if (cmp == 0) return racine;
    if (cmp < 0) return rechercherRacine(racine->gauche, cle);
    return rechercherRacine(racine->droite, cle);
}

void afficherRacines(NoeudArbre* racine) {
    if (racine == NULL) return;
    afficherRacines(racine->gauche);
    printf("%s\n", racine->data.racine);
    afficherRacines(racine->droite);
}

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
