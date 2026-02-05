#ifndef HASH_H
#define HASH_H

#include "types.h"

// Crée une table de hachage vide
TableHash* creerTableHash();

// Fonction de hachage (interne)
unsigned int hacher(const char* cle);

// Insère un schème dans la table
void insererScheme(TableHash* t, Scheme s);

// Cherche un schème par nom, retourne pointeur ou NULL
Scheme* chercherScheme(TableHash* t, const char* nom);

// Supprime un schème (optionnel pour début)
void supprimerScheme(TableHash* t, const char* nom);

// Affiche tous les schèmes de la table
void afficherSchemes(const TableHash* t);

// Libère la table de hachage
void libererTableHash(TableHash* t);

#endif
