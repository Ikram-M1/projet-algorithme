#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "morphologie.h"

void ajouterOuIncrementerDerive(Racine* r, const char* mot) {
    Derive* d = r->derives;
    while (d) {
        if (strcmp(d->mot, mot) == 0) {
            d->frequence++;
            return;
        }
        d = d->suivant;
    }
    Derive* nouveau = (Derive*)malloc(sizeof(Derive));
    if (!nouveau) return;
    strncpy(nouveau->mot, mot, MAX_LEN - 1);
    nouveau->mot[MAX_LEN - 1] = '\0';
    nouveau->frequence = 1;
    nouveau->suivant = r->derives;
    r->derives = nouveau;
}

void afficherDerivesRacine(const Racine* r) {
    Derive* d = r->derives;
    if (!d) {
        printf("Aucun dérivé pour la racine %s.\n", r->racine);
        return;
    }
    printf("Dérivés pour la racine %s:\n", r->racine);
    while (d) {
        printf(" - %s (freq: %d)\n", d->mot, d->frequence);
        d = d->suivant;
    }
}

// Ici on suppose que racine est une chaîne de 3 "lettres" logiques.
// En pratique avec l'arabe/UTF-8, tu adapteras.
char* genererMot(const char* racine, const Scheme* sch) {
    char r1 = racine[0];
    char r2 = racine[1];
    char r3 = racine[2];

    char buffer[2 * MAX_LEN];
    int k = 0;

    for (int i = 0; sch->pattern[i] != '\0'; ++i) {
        char c = sch->pattern[i];
        if (c == '1') buffer[k++] = r1;
        else if (c == '2') buffer[k++] = r2;
        else if (c == '3') buffer[k++] = r3;
        else buffer[k++] = c;
    }
    buffer[k] = '\0';

    char* res = (char*)malloc((k + 1) * sizeof(char));
    if (!res) return NULL;
    strcpy(res, buffer);
    return res;
}

void genererFamilleMorphologique(NoeudArbre* arbre, TableHash* t, const char* racineStr) {
    NoeudArbre* n = rechercherRacine(arbre, racineStr);
    if (!n) {
        printf("Racine %s introuvable.\n", racineStr);
        return;
    }
    printf("Famille morphologique pour la racine %s:\n", racineStr);
    for (int i = 0; i < TAILLE_TABLE; ++i) {
        EntreeHash* e = t->cases[i];
        while (e) {
            char* mot = genererMot(racineStr, &e->valeur);
            if (mot) {
                printf("Schème: %s -> %s\n", e->valeur.nom, mot);
                ajouterOuIncrementerDerive(&n->data, mot);
                free(mot);
            }
            e = e->suivant;
        }
    }
}

int validerMotPourRacine(TableHash* t, const char* mot, const char* racineStr, Scheme** schTrouve) {
    if (schTrouve) *schTrouve = NULL;
    for (int i = 0; i < TAILLE_TABLE; ++i) {
        EntreeHash* e = t->cases[i];
        while (e) {
            char* candidate = genererMot(racineStr, &e->valeur);
            if (candidate && strcmp(candidate, mot) == 0) {
                if (schTrouve) *schTrouve = &e->valeur;
                free(candidate);
                return 1;
            }
            if (candidate) free(candidate);
            e = e->suivant;
        }
    }
    return 0;
}
