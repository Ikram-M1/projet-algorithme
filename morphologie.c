#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "morphologie.h"

Scheme creerScheme(const char* nom, const char* pattern) {
    Scheme s;
    strncpy(s.nom, nom, MAX_LEN - 1);
    s.nom[MAX_LEN - 1] = '\0';
    strncpy(s.pattern, pattern, MAX_LEN - 1);
    s.pattern[MAX_LEN - 1] = '\0';
    return s;
}

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

char* genererMot(const char* racine, const Scheme* sch) {
    // FIX UTF-8 ARABE : chaque lettre = 2 octets (ك=2, ت=2, ب=2 → strlen=6)
    if (strlen(racine) != 6) return NULL;  // Trilitère stricte
    
    char r1[3] = {racine[0], racine[1], '\0'};  // ك
    char r2[3] = {racine[2], racine[3], '\0'};  // ت  
    char r3[3] = {racine[4], racine[5], '\0'};  // ب

    char buffer[4 * MAX_LEN] = {0};
    int k = 0;

    for (int i = 0; sch->pattern[i]; i++) {
        char c = sch->pattern[i];
        if (c == '1') { strcpy(buffer+k, r1); k += 2; }
        else if (c == '2') { strcpy(buffer+k, r2); k += 2; }
        else if (c == '3') { strcpy(buffer+k, r3); k += 2; }
        else { buffer[k++] = c; }
    }
    buffer[k] = '\0';

    char* res = malloc(strlen(buffer) + 1);
    if (res) strcpy(res, buffer);
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

int validerMotPourRacine(NoeudArbre* arbre, TableHash* t, const char* mot, const char* racineStr, Scheme** schTrouve) {
    if (schTrouve) *schTrouve = NULL;
    for (int i = 0; i < TAILLE_TABLE; ++i) {
        EntreeHash* e = t->cases[i];
        while (e) {
            char* candidate = genererMot(racineStr, &e->valeur);
            if (candidate && strcmp(candidate, mot) == 0) {
                if (schTrouve) *schTrouve = &e->valeur;
                NoeudArbre* n = rechercherRacine(arbre, racineStr);
                if (n) ajouterOuIncrementerDerive(&n->data, mot);  // Mise à jour auto
                free(candidate);
                return 1;
            }
            if (candidate) free(candidate);
            e = e->suivant;
        }
    }
    return 0;
}