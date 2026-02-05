#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash.h"

TableHash* creerTableHash() {
    TableHash* t = (TableHash*)malloc(sizeof(TableHash));
    if (!t) {
        fprintf(stderr, "Erreur d'allocation pour la table de hachage.\n");
        return NULL;
    }
    for (int i = 0; i < TAILLE_TABLE; ++i) {
        t->cases[i] = NULL;
    }
    return t;
}

// Fonction de hachage simple pour une chaîne (UTF-8 traité au niveau octet)
static unsigned int hacher(const char* cle) {
    unsigned long h = 0;
    while (*cle) {
        h = (h * 31u + (unsigned char)(*cle)) % TAILLE_TABLE;
        cle++;
    }
    return (unsigned int)h;
}

void insererScheme(TableHash* t, Scheme s) {
    if (!t) return;
    unsigned int idx = hacher(s.nom);

    // Vérifier si un schème avec ce nom existe déjà -> on le remplace
    EntreeHash* e = t->cases[idx];
    while (e) {
        if (strcmp(e->cle, s.nom) == 0) {
            e->valeur = s;   // mise à jour du schème existant
            return;
        }
        e = e->suivant;
    }

    // Sinon, insérer une nouvelle entrée en tête de liste (chaînage)
    EntreeHash* nouveau = (EntreeHash*)malloc(sizeof(EntreeHash));
    if (!nouveau) {
        fprintf(stderr, "Erreur d'allocation pour une entrée de schème.\n");
        return;
    }
    strncpy(nouveau->cle, s.nom, MAX_LEN - 1);
    nouveau->cle[MAX_LEN - 1] = '\0';
    nouveau->valeur = s;
    nouveau->suivant = t->cases[idx];
    t->cases[idx] = nouveau;
}

Scheme* chercherScheme(TableHash* t, const char* nom) {
    if (!t) return NULL;
    unsigned int idx = hacher(nom);
    EntreeHash* e = t->cases[idx];
    while (e) {
        if (strcmp(e->cle, nom) == 0) {
            return &e->valeur;
        }
        e = e->suivant;
    }
    return NULL;
}

void supprimerScheme(TableHash* t, const char* nom) {
    if (!t) return;
    unsigned int idx = hacher(nom);
    EntreeHash* e = t->cases[idx];
    EntreeHash* prev = NULL;

    while (e) {
        if (strcmp(e->cle, nom) == 0) {
            if (prev) prev->suivant = e->suivant;
            else t->cases[idx] = e->suivant;
            free(e);
            return;
        }
        prev = e;
        e = e->suivant;
    }
}

void afficherSchemes(const TableHash* t) {
    if (!t) return;
    printf("Schèmes disponibles :\n");
    for (int i = 0; i < TAILLE_TABLE; ++i) {
        EntreeHash* e = t->cases[i];
        while (e) {
            printf(" - %s | pattern: %s\n", e->valeur.nom, e->valeur.pattern);
            e = e->suivant;
        }
    }
}

void libererTableHash(TableHash* t) {
    if (!t) return;
    for (int i = 0; i < TAILLE_TABLE; ++i) {
        EntreeHash* e = t->cases[i];
        while (e) {
            EntreeHash* tmp = e;
            e = e->suivant;
            free(tmp);
        }
    }
    free(t);
}
