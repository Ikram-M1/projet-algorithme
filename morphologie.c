#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "morphologie.h"
#include "arbre.h"

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
    printf("Dérivés validés pour la racine %s:\n", r->racine);
    printf("========================================\n");
    int count = 0;
    while (d) {
        printf("  %d. %s (fréquence: %d)\n", ++count, d->mot, d->frequence);
        d = d->suivant;
    }
    printf("========================================\n");
    printf("Total: %d dérivé(s)\n", count);
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

/**
 * Tente de décomposer un mot donné en racine + schème
 * Retourne 1 si trouvé, 0 sinon
 * racineTrouvee et schemeTrouve doivent être des buffers de taille MAX_LEN
 */
int decomposerMot(NoeudArbre* arbre, TableHash* schemes, const char* mot,
                  char* racineTrouvee, char* schemeTrouve)
{
    if (!mot || !*mot || !arbre || !schemes) {
        return 0;
    }

    // On va parcourir toutes les racines existantes
    // (cette approche est acceptable si le nombre de racines reste < 5000–10000)
    int count = compterNoeuds(arbre);
    if (count == 0) return 0;

    const char** racines = malloc(count * sizeof(const char*));
    if (!racines) return 0;

    int idx = 0;
    remplirTableau(arbre, racines, &idx);

    // Pas besoin de trier ici → on teste toutes les racines
    for (int i = 0; i < count; i++) {
        const char* racine_cand = racines[i];

        // Pour chaque racine candidate, on teste tous les schèmes
        for (int j = 0; j < TAILLE_TABLE; j++) {
            EntreeHash* e = schemes->cases[j];
            while (e) {
                char* genere = genererMot(racine_cand, &e->valeur);
                if (genere && strcmp(genere, mot) == 0) {
                    // On a trouvé !
                    strncpy(racineTrouvee, racine_cand, MAX_LEN - 1);
                    racineTrouvee[MAX_LEN - 1] = '\0';
                    
                    strncpy(schemeTrouve, e->valeur.nom, MAX_LEN - 1);
                    schemeTrouve[MAX_LEN - 1] = '\0';

                    free(genere);
                    free(racines);
                    return 1;
                }
                if (genere) free(genere);
                e = e->suivant;
            }
        }
    }

    free(racines);
    return 0;
}

// ============================================
// SECTION VERBES IRRÉGULIERS - CREUX (AJWAF)
// ============================================

/**
 * Détecte si une racine est de type Ajwaf (creux)
 * Retourne 1 si R2 (deuxième lettre) est و ou ي, 0 sinon
 */
int estAjwaf(const char* racine) {
    if (!racine || strlen(racine) != 6) return 0;  // UTF-8: 3 lettres = 6 octets
    
    // R2 = racine[2] et racine[3]
    // و (waw) = {0xD9, 0x88} ou {0xD9, 0x88}
    // ي (ya) = {0xD9, 0x8A} ou {0xD9, 0x8A}
    
    // Simpler: Compare avec les chaînes connues
    char waw[3] = {0xD9, 0x88, 0}; // و
    char ya[3] = {0xD9, 0x8A, 0};  // ي
    
    char r2[3] = {racine[2], racine[3], '\0'};
    
    return (strcmp(r2, waw) == 0 || strcmp(r2, ya) == 0);
}

/**
 * Génère un mot à partir d'une racine Ajwaf et d'un schème
 * Gère les cas spéciaux où R2 (و ou ي) est remplacé par ا ou par hamza
 * Exemple: عيش (3-ي-sh) + fa3ala pattern "1ا23" → عائش (pas عاايش)
 * Règle: quand ا2, remplacer R2 par hamza (ء)
 */
char* genererMotAjwaf(const char* racine, const Scheme* sch) {
    if (!racine || strlen(racine) != 6) return NULL;  // UTF-8: 3 lettres = 6 octets
    
    char r1[3] = {racine[0], racine[1], '\0'};  // R1
    char r2[3] = {racine[2], racine[3], '\0'};  // R2 (و ou ي)
    char r3[3] = {racine[4], racine[5], '\0'};  // R3
    
    // ا (alif) en UTF-8 = {0xD8, 0xA7}
    char alif[3] = {0xD8, 0xA7, '\0'};
    
    // ء (hamza) en UTF-8 = {0xD8, 0xA1}
    char hamza[3] = {0xD8, 0xA1, '\0'};
    
    // Parcourir le pattern et détecter deux cas possibles pour le remplacement de R2:
    //  - si on trouve "ا2" => remplacer R2 par hamza (ء)
    //  - si on trouve "ت2" ou si le nom du schème est "افتعل" => remplacer R2 par alif (ا)
    int replaceWithHamza = 0;
    int replaceWithAlif = 0;
    int len = strlen(sch->pattern);

    for (int i = 0; i < len - 2; i++) {
        // ا (alif) en UTF-8 = D8 A7 ; ت (ta) en UTF-8 = D8 AA
        if (sch->pattern[i] == (char)0xD8 && sch->pattern[i+1] == (char)0xA7 && sch->pattern[i+2] == '2') {
            replaceWithHamza = 1;
            break;
        }
        if (sch->pattern[i] == (char)0xD8 && sch->pattern[i+1] == (char)0xAA && sch->pattern[i+2] == '2') {
            replaceWithAlif = 1;
            break;
        }
    }

    // Si le nom du schème est exactement "افتعل", forcer le comportement en alif
    if (!replaceWithAlif && strcmp(sch->nom, "افتعل") == 0) {
        replaceWithAlif = 1;
    }
    
    char buffer[4 * MAX_LEN] = {0};
    int k = 0;
    
    for (int i = 0; i < len; i++) {
        char c = sch->pattern[i];
        
        if (c == '1') {
            strcpy(buffer + k, r1);
            k += 2;
        }
        else if (c == '2') {
            if (replaceWithHamza) {
                strcpy(buffer + k, hamza);
                k += 2;
            } else if (replaceWithAlif) {
                strcpy(buffer + k, alif);
                k += 2;
            } else {
                strcpy(buffer + k, r2);
                k += 2;
            }
        }
        else if (c == '3') {
            strcpy(buffer + k, r3);
            k += 2;
        }
        else {
            buffer[k++] = c;
        }
    }
    buffer[k] = '\0';
    
    char* res = malloc(strlen(buffer) + 1);
    if (res) strcpy(res, buffer);
    return res;
}

/**
 * Génère et affiche tous les dérivés d'une racine Ajwaf pour tous les schèmes
 */
void genererFamilleAjwaf(NoeudArbre* arbre, TableHash* t, const char* racineStr) {
    NoeudArbre* n = rechercherRacine(arbre, racineStr);
    if (!n) {
        printf("Racine %s introuvable.\n", racineStr);
        return;
    }
    
    if (!estAjwaf(racineStr)) {
        printf("✗ La racine %s n'est pas de type Creux (Ajwaf).\n", racineStr);
        return;
    }
    
    printf("Famille morphologique pour la racine Ajwaf %s:\n", racineStr);
    for (int i = 0; i < TAILLE_TABLE; ++i) {
        EntreeHash* e = t->cases[i];
        while (e) {
            char* mot = genererMotAjwaf(racineStr, &e->valeur);
            if (mot) {
                printf("  Schème: %s → %s\n", e->valeur.nom, mot);
                ajouterOuIncrementerDerive(&n->data, mot);
                free(mot);
            }
            e = e->suivant;
        }
    }
}

/**
 * Vérifie si un mot appartient à une racine Ajwaf
 */
int validerMotPourRacineAjwaf(NoeudArbre* arbre, TableHash* t, const char* mot, 
                              const char* racineStr, Scheme** schTrouve) {
    if (schTrouve) *schTrouve = NULL;
    
    if (!estAjwaf(racineStr)) {
        printf("✗ La racine %s n'est pas de type Creux (Ajwaf).\n", racineStr);
        return 0;
    }
    
    for (int i = 0; i < TAILLE_TABLE; ++i) {
        EntreeHash* e = t->cases[i];
        while (e) {
            char* candidate = genererMotAjwaf(racineStr, &e->valeur);
            if (candidate && strcmp(candidate, mot) == 0) {
                if (schTrouve) *schTrouve = &e->valeur;
                NoeudArbre* n = rechercherRacine(arbre, racineStr);
                if (n) ajouterOuIncrementerDerive(&n->data, mot);
                free(candidate);
                return 1;
            }
            if (candidate) free(candidate);
            e = e->suivant;
        }
    }
    
    return 0;
}

/**
 * Décompose un mot pour en extraire la racine Ajwaf et le schème
 */
int decomposerMotAjwaf(NoeudArbre* arbre, TableHash* schemes, const char* mot,
                       char* racineTrouvee, char* schemeTrouve)
{
    if (!mot || !*mot || !arbre || !schemes) {
        return 0;
    }

    int count = compterNoeuds(arbre);
    if (count == 0) return 0;

    const char** racines = malloc(count * sizeof(const char*));
    if (!racines) return 0;

    int idx = 0;
    remplirTableau(arbre, racines, &idx);

    for (int i = 0; i < count; i++) {
        const char* racine_cand = racines[i];
        
        // Vérifier uniquement les racines Ajwaf
        if (!estAjwaf(racine_cand)) continue;

        for (int j = 0; j < TAILLE_TABLE; j++) {
            EntreeHash* e = schemes->cases[j];
            while (e) {
                char* genere = genererMotAjwaf(racine_cand, &e->valeur);
                if (genere && strcmp(genere, mot) == 0) {
                    strncpy(racineTrouvee, racine_cand, MAX_LEN - 1);
                    racineTrouvee[MAX_LEN - 1] = '\0';
                    
                    strncpy(schemeTrouve, e->valeur.nom, MAX_LEN - 1);
                    schemeTrouve[MAX_LEN - 1] = '\0';

                    free(genere);
                    free(racines);
                    return 1;
                }
                if (genere) free(genere);
                e = e->suivant;
            }
        }
    }

    free(racines);
    return 0;
}