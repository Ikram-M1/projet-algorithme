#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "morphologie.h"

// -----------------------------------------------------------------------------
// Fonctions utilitaires UTF-8 pour l'arabe
// -----------------------------------------------------------------------------

/**
 * Compte le nombre de lettres arabes dans une chaîne UTF-8
 * (chaque lettre arabe = 2 octets en UTF-8)
 */
static int compterLettresArabes(const char* str) {
    if (!str) return 0;
    
    int count = 0;
    const unsigned char* p = (const unsigned char*)str;
    
    while (*p) {
        // Détection UTF-8 arabe : octets commençant par 0xD8 ou 0xD9
        if ((*p == 0xD8 || *p == 0xD9) && *(p+1) >= 0x80) {
            count++;
            p += 2;  // Avancer de 2 octets
        } else {
            p++;
        }
    }
    
    return count;
}

/**
 * Extrait la n-ième lettre arabe d'une chaîne UTF-8 (0-indexed)
 * Stocke le résultat dans buffer (doit être de taille >= 3)
 */
static int extraireLettreArabe(const char* str, int index, char* buffer) {
    if (!str || !buffer || index < 0) return 0;
    
    const unsigned char* p = (const unsigned char*)str;
    int count = 0;
    
    while (*p) {
        if ((*p == 0xD8 || *p == 0xD9) && *(p+1) >= 0x80) {
            if (count == index) {
                buffer[0] = p[0];
                buffer[1] = p[1];
                buffer[2] = '\0';
                return 1;
            }
            count++;
            p += 2;
        } else {
            p++;
        }
    }
    
    return 0;  // Index hors limites
}

/**
 * Vérifie si une chaîne est une racine trilitère arabe valide
 */
static int estRacineValide(const char* racine) {
    if (!racine) return 0;
    
    // Une racine trilitère = exactement 3 lettres arabes = 6 octets UTF-8
    int nbLettres = compterLettresArabes(racine);
    return (nbLettres == 3 && strlen(racine) == 6);
}

/**
 * Nettoie une chaîne (supprime espaces début/fin)
 */
static void trimString(char* str) {
    if (!str) return;
    
    char* start = str;
    while (*start && isspace((unsigned char)*start)) {
        start++;
    }
    
    char* end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) {
        end--;
    }
    *(end + 1) = '\0';
    
    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }
}

// -----------------------------------------------------------------------------
// Création de schèmes
// -----------------------------------------------------------------------------

Scheme creerScheme(const char* nom, const char* pattern) {
    Scheme s;
    
    if (!nom || !pattern) {
        fprintf(stderr, "❌ Erreur : nom ou pattern NULL\n");
        s.nom[0] = '\0';
        s.pattern[0] = '\0';
        s.regleTransformation = NULL;
        return s;
    }
    
    strncpy(s.nom, nom, MAX_LEN - 1);
    s.nom[MAX_LEN - 1] = '\0';
    trimString(s.nom);
    
    strncpy(s.pattern, pattern, MAX_LEN - 1);
    s.pattern[MAX_LEN - 1] = '\0';
    trimString(s.pattern);
    
    s.regleTransformation = NULL;  // À définir si nécessaire
    
    return s;
}

// -----------------------------------------------------------------------------
// Gestion des dérivés (utilisé par arbre.c aussi)
// -----------------------------------------------------------------------------

void ajouterOuIncrementerDerive(Racine* r, const char* mot) {
    if (!r || !mot) {
        fprintf(stderr, "❌ Erreur : racine ou mot NULL\n");
        return;
    }

    // Nettoyage du mot
    char motClean[MAX_LEN];
    strncpy(motClean, mot, MAX_LEN - 1);
    motClean[MAX_LEN - 1] = '\0';
    trimString(motClean);

    if (strlen(motClean) == 0) {
        fprintf(stderr, "❌ Erreur : mot vide après nettoyage\n");
        return;
    }

    // Recherche si déjà présent
    Derive* d = r->derives;
    while (d) {
        if (strcmp(d->mot, motClean) == 0) {
            d->frequence++;
            return;
        }
        d = d->suivant;
    }

    // Ajout nouveau dérivé
    Derive* nouveau = (Derive*)malloc(sizeof(Derive));
    if (!nouveau) {
        fprintf(stderr, "❌ Erreur : allocation mémoire échouée\n");
        return;
    }

    strcpy(nouveau->mot, motClean);
    nouveau->frequence = 1;
    nouveau->suivant = r->derives;
    r->derives = nouveau;
}

// -----------------------------------------------------------------------------
// Affichage des dérivés
// -----------------------------------------------------------------------------

void afficherDerivesRacineSimple(const Racine* r) {
    if (!r) {
        printf("⚠️  Racine NULL\n");
        return;
    }

    Derive* d = r->derives;
    if (!d) {
        printf("\n⚠️  Aucun dérivé pour la racine '%s'\n\n", r->racine);
        return;
    }

    printf("\n╔═══════════════════════════════════════════════════╗\n");
    printf("║   📖 Dérivés de la racine : %s\n", r->racine);
    printf("╚═══════════════════════════════════════════════════╝\n\n");

    int count = 1;
    while (d) {
        printf("  %3d. %-30s (fréquence: %d)\n", count, d->mot, d->frequence);
        d = d->suivant;
        count++;
    }
    printf("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("📊 Total : %d dérivé(s)\n\n", count - 1);
}

// Version callback (pour cohérence avec arbre.c et hash.c)
// NOTE: Cette fonction n'est plus utilisée ici, on utilise celle de arbre.c
// qui prend un NoeudArbre* au lieu d'une Racine*

// -----------------------------------------------------------------------------
// Génération morphologique (cœur du moteur)
// -----------------------------------------------------------------------------

/**
 * Génère un mot dérivé à partir d'une racine et d'un schème.
 * Pattern : utilise 1, 2, 3 pour substituer les consonnes de la racine
 * 
 * Exemple : racine = "كتب" (k-t-b), pattern = "1ا2ِ3" → "كاتِب" (katib)
 */
char* genererMot(const char* racine, const Scheme* sch) {
    if (!racine || !sch) {
        fprintf(stderr, "❌ Erreur : racine ou schème NULL\n");
        return NULL;
    }

    // Validation : racine trilitère stricte
    if (!estRacineValide(racine)) {
        fprintf(stderr, "❌ Erreur : '%s' n'est pas une racine trilitère valide\n", racine);
        return NULL;
    }

    // Extraction des 3 consonnes radicales
    char r1[3], r2[3], r3[3];
    if (!extraireLettreArabe(racine, 0, r1) ||
        !extraireLettreArabe(racine, 1, r2) ||
        !extraireLettreArabe(racine, 2, r3)) {
        fprintf(stderr, "❌ Erreur : extraction des lettres échouée\n");
        return NULL;
    }

    // Buffer suffisamment grand pour le résultat
    char buffer[4 * MAX_LEN] = {0};
    int k = 0;

    // Parcours du pattern et substitution
    for (int i = 0; sch->pattern[i]; i++) {
        unsigned char c = (unsigned char)sch->pattern[i];
        
        if (c == '1') {
            strcpy(buffer + k, r1);
            k += 2;
        } else if (c == '2') {
            strcpy(buffer + k, r2);
            k += 2;
        } else if (c == '3') {
            strcpy(buffer + k, r3);
            k += 2;
        } else if (c >= 0xD8 && c <= 0xD9) {
            // Caractère arabe (2 octets UTF-8)
            buffer[k++] = sch->pattern[i];
            if (sch->pattern[i+1]) {
                buffer[k++] = sch->pattern[++i];
            }
        } else {
            // Autre caractère (ASCII, voyelles courtes, etc.)
            buffer[k++] = c;
        }
    }
    buffer[k] = '\0';

    // Allocation et copie du résultat
    char* res = malloc(strlen(buffer) + 1);
    if (!res) {
        fprintf(stderr, "❌ Erreur : allocation mémoire échouée\n");
        return NULL;
    }
    
    strcpy(res, buffer);
    return res;
}

// -----------------------------------------------------------------------------
// Génération de famille morphologique
// -----------------------------------------------------------------------------

/**
 * Génère tous les dérivés possibles pour une racine donnée
 * en appliquant tous les schèmes disponibles dans la table.
 */
void genererFamilleMorphologique(NoeudArbre* arbre, TableHash* t, const char* racineStr) {
    if (!arbre || !t || !racineStr) {
        printf("❌ Erreur : paramètres invalides\n");
        return;
    }

    // Recherche de la racine dans l'arbre
    NoeudArbre* n = rechercherRacine(arbre, racineStr);
    if (!n) {
        printf("❌ Racine '%s' introuvable dans l'arbre.\n", racineStr);
        return;
    }

    printf("\n╔═══════════════════════════════════════════════════════════╗\n");
    printf("║   🔄 GÉNÉRATION FAMILLE MORPHOLOGIQUE                    ║\n");
    printf("╚═══════════════════════════════════════════════════════════╝\n");
    printf("\n📌 Racine : %s\n", racineStr);
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    int count = 0;
    int erreurs = 0;

    // Parcours de tous les schèmes
    for (int i = 0; i < TAILLE_TABLE; ++i) {
        EntreeHash* e = t->cases[i];
        while (e) {
            char* mot = genererMot(racineStr, &e->valeur);
            if (mot) {
                printf("  ✓ Schème %-12s → %s\n", e->valeur.nom, mot);
                ajouterDerive(n, mot);  // Utilise la fonction d'arbre.c
                free(mot);
                count++;
            } else {
                erreurs++;
            }
            e = e->suivant;
        }
    }

    printf("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("📊 Résultats : %d dérivé(s) généré(s)", count);
    if (erreurs > 0) {
        printf(" (%d erreur(s))", erreurs);
    }
    printf("\n\n");
}

// Version callback (pour interface cohérente)
void genererFamilleMorphologiqueCallback(NoeudArbre* arbre, TableHash* t, 
                                         const char* racineStr, AppendFunc append) {
    if (!append || !arbre || !t || !racineStr) return;

    NoeudArbre* n = rechercherRacine(arbre, racineStr);
    if (!n) {
        char msg[200];
        snprintf(msg, sizeof(msg), "❌ Racine '%s' introuvable.\n", racineStr);
        append(msg);
        return;
    }

    char line[300];
    snprintf(line, sizeof(line),
             "\n╔═══════════════════════════════════════════════════════════╗\n"
             "║   🔄 GÉNÉRATION FAMILLE MORPHOLOGIQUE                    ║\n"
             "╚═══════════════════════════════════════════════════════════╝\n"
             "\n📌 Racine : %s\n"
             "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n",
             racineStr);
    append(line);

    int count = 0;
    for (int i = 0; i < TAILLE_TABLE; ++i) {
        EntreeHash* e = t->cases[i];
        while (e) {
            char* mot = genererMot(racineStr, &e->valeur);
            if (mot) {
                snprintf(line, sizeof(line), "  ✓ Schème %-12s → %s\n", 
                         e->valeur.nom, mot);
                append(line);
                ajouterDerive(n, mot);
                free(mot);
                count++;
            }
            e = e->suivant;
        }
    }

    snprintf(line, sizeof(line),
             "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n"
             "📊 %d dérivé(s) généré(s)\n\n",
             count);
    append(line);
}

// -----------------------------------------------------------------------------
// Validation morphologique
// -----------------------------------------------------------------------------

/**
 * Vérifie si un mot appartient morphologiquement à une racine.
 * Teste tous les schèmes disponibles et compare les résultats.
 * 
 * @return 1 si validation réussie, 0 sinon
 * @param schTrouve Si non-NULL, stocke le pointeur vers le schème trouvé
 */
int validerMotPourRacine(NoeudArbre* arbre, TableHash* t, const char* mot, 
                         const char* racineStr, Scheme** schTrouve) {
    if (!arbre || !t || !mot || !racineStr) {
        fprintf(stderr, "❌ Erreur : paramètres invalides\n");
        return 0;
    }

    if (schTrouve) *schTrouve = NULL;

    // Nettoyage du mot à valider
    char motClean[MAX_LEN];
    strncpy(motClean, mot, MAX_LEN - 1);
    motClean[MAX_LEN - 1] = '\0';
    trimString(motClean);

    // Test avec tous les schèmes
    for (int i = 0; i < TAILLE_TABLE; ++i) {
        EntreeHash* e = t->cases[i];
        while (e) {
            char* candidate = genererMot(racineStr, &e->valeur);
            if (candidate && strcmp(candidate, motClean) == 0) {
                // Match trouvé !
                if (schTrouve) {
                    *schTrouve = &e->valeur;
                }
                
                // Mise à jour automatique de l'arbre
                NoeudArbre* n = rechercherRacine(arbre, racineStr);
                if (n) {
                    ajouterDerive(n, motClean);
                }
                
                free(candidate);
                return 1;  // Validation réussie
            }
            if (candidate) free(candidate);
            e = e->suivant;
        }
    }

    return 0;  // Aucun schème ne correspond
}

/**
 * Extrait la racine d'un mot (inverse de genererMot - heuristique)
 * ATTENTION : Fonction expérimentale, peut ne pas fonctionner pour tous les schèmes
 */
char* extraireRacine(const char* mot, const Scheme* sch) {
    if (!mot || !sch) return NULL;

    // Implémentation basique : cherche les positions 1, 2, 3 dans le pattern
    // et extrait les lettres correspondantes du mot
    // TODO : Implémentation complète selon les besoins du projet
    
    fprintf(stderr, "⚠️  Fonction extraireRacine() non implémentée\n");
    return NULL;
}