#ifndef MORPHOLOGIE_H
#define MORPHOLOGIE_H

#include "types.h"
#include "arbre.h"
#include "hash.h"

// Crée un schème à partir d'un nom (ex: "مفعول") et d'un pattern (ex: "م12و3")
Scheme creerScheme(const char* nom, const char* pattern);

// Ajoute ou incrémente un dérivé pour une racine
void ajouterOuIncrementerDerive(Racine* r, const char* mot);

// Affiche les dérivés d'une racine
void afficherDerivesRacine(const Racine* r);

// Génère un mot à partir d'une racine et d'un schème
// racine est supposée avoir 3 "consonnes" logiques (ex: "كتب")
char* genererMot(const char* racine, const Scheme* sch);

// Génère et affiche tous les dérivés d'une racine pour tous les schèmes de la table
void genererFamilleMorphologique(NoeudArbre* arbre, TableHash* t, const char* racine);

// Vérifie si un mot appartient à une racine (OUI/NON)
// Retourne 1 si oui, 0 sinon, et met *schTrouve sur le schème reconnu si trouvé
int validerMotPourRacine(NoeudArbre* arbre, TableHash* t, const char* mot, const char* racine, Scheme** schTrouve);

#endif