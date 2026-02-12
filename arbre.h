#ifndef ARBRE_H
#define ARBRE_H

#include "types.h"

// Crée un nouveau noeud avec une racine donnée
NoeudArbre* creerNoeud(const char* racine);

// Insère une racine dans l'arbre (ABR simple)
// Retourne la nouvelle racine de l'arbre (utile pour l'appel récursif)
NoeudArbre* insererRacine(NoeudArbre* racine, const char* nouvelleRacine);

// Recherche une racine dans l'arbre
NoeudArbre* rechercherRacine(NoeudArbre* racine, const char* cle);

// Affiche toutes les racines (parcours infixe)
void afficherRacines(NoeudArbre* racine);

// Libère toute la mémoire de l'arbre
void libererArbre(NoeudArbre* racine);

const char* selectionnerRacineParNumero(NoeudArbre* racine);

int compterNoeuds(NoeudArbre *n);
void remplirTableau(NoeudArbre *n, const char **tab, int *idx);
#endif