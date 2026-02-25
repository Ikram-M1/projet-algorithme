#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "hash.h"

// -----------------------------------------------------------------------------
// Création / Initialisation
// -----------------------------------------------------------------------------

TableHash* creerTableHash() {
    TableHash* t = (TableHash*)malloc(sizeof(TableHash));
    if (!t) {
        fprintf(stderr, "❌ Erreur : allocation mémoire échouée pour la table de hachage\n");
        return NULL;
    }

    // Initialisation de toutes les cases à NULL
    for (int i = 0; i < TAILLE_TABLE; i++) {
        t->cases[i] = NULL;
    }
    t->nbSchemes = 0;
    
    return t;
}

// -----------------------------------------------------------------------------
// Fonction de hachage (DJB2 - efficace pour UTF-8)
// -----------------------------------------------------------------------------

/**
 * Fonction de hachage DJB2 (Daniel J. Bernstein)
 * Excellente distribution pour les chaînes courtes et UTF-8
 * Complexité : O(m) où m = longueur de la clé
 */
static unsigned int hacher(const char* cle) {
    if (!cle) return 0;
    
    unsigned long h = 5381;  // Constante magique DJB2
    int c;
    
    // Traite chaque octet (compatible UTF-8)
    while ((c = (unsigned char)*cle++)) {
        h = ((h << 5) + h) + c;   // h * 33 + c
    }
    
    return (unsigned int)(h % TAILLE_TABLE);
}

// -----------------------------------------------------------------------------
// Fonctions utilitaires internes
// -----------------------------------------------------------------------------

/**
 * Nettoie une chaîne (supprime espaces début/fin)
 */
static void trimString(char* str) {
    if (!str) return;
    
    // Supprimer espaces à gauche
    char* start = str;
    while (*start && isspace((unsigned char)*start)) {
        start++;
    }
    
    // Supprimer espaces à droite
    char* end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) {
        end--;
    }
    *(end + 1) = '\0';
    
    // Déplacer si nécessaire
    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }
}

/**
 * Valide un nom de schème (non vide après nettoyage)
 */
static int estNomValide(const char* nom) {
    if (!nom || strlen(nom) == 0) return 0;
    
    char temp[MAX_LEN];
    strncpy(temp, nom, MAX_LEN - 1);
    temp[MAX_LEN - 1] = '\0';
    trimString(temp);
    
    return strlen(temp) > 0;
}

// -----------------------------------------------------------------------------
// Insertion / Mise à jour d'un schème
// -----------------------------------------------------------------------------

void insererScheme(TableHash* t, Scheme s) {
    if (!t) {
        fprintf(stderr, "❌ Erreur : table de hachage NULL\n");
        return;
    }

    // Validation du nom du schème
    if (!estNomValide(s.nom)) {
        fprintf(stderr, "❌ Erreur : nom de schème invalide\n");
        return;
    }

    // Nettoyage des champs
    trimString(s.nom);
    trimString(s.pattern);

    unsigned int idx = hacher(s.nom);
    EntreeHash* e = t->cases[idx];

    // Recherche si existe déjà → mise à jour
    while (e) {
        if (strcmp(e->cle, s.nom) == 0) {
            // Mise à jour du schème existant
            e->valeur = s;
            return;
        }
        e = e->suivant;
    }

    // Nouvelle entrée (insertion en tête)
    EntreeHash* nouveau = (EntreeHash*)malloc(sizeof(EntreeHash));
    if (!nouveau) {
        fprintf(stderr, "❌ Erreur : allocation mémoire échouée pour l'entrée\n");
        return;
    }

    strncpy(nouveau->cle, s.nom, MAX_LEN - 1);
    nouveau->cle[MAX_LEN - 1] = '\0';
    nouveau->valeur = s;
    nouveau->suivant = t->cases[idx];
    t->cases[idx] = nouveau;

    t->nbSchemes++;
}

// -----------------------------------------------------------------------------
// Recherche d'un schème par nom
// -----------------------------------------------------------------------------

Scheme* chercherScheme(const TableHash* t, const char* nom) {
    if (!t || !nom) return NULL;

    // Nettoyage du nom recherché
    char nomClean[MAX_LEN];
    strncpy(nomClean, nom, MAX_LEN - 1);
    nomClean[MAX_LEN - 1] = '\0';
    trimString(nomClean);

    unsigned int idx = hacher(nomClean);
    EntreeHash* e = ((TableHash*)t)->cases[idx];

    while (e) {
        if (strcmp(e->cle, nomClean) == 0) {
            return &e->valeur;
        }
        e = e->suivant;
    }
    
    return NULL;
}

// -----------------------------------------------------------------------------
// Suppression d'un schème par nom
// -----------------------------------------------------------------------------

int supprimerScheme(TableHash* t, const char* nom) {
    if (!t || !nom) return 0;

    // Nettoyage du nom
    char nomClean[MAX_LEN];
    strncpy(nomClean, nom, MAX_LEN - 1);
    nomClean[MAX_LEN - 1] = '\0';
    trimString(nomClean);

    unsigned int idx = hacher(nomClean);
    EntreeHash* e = t->cases[idx];
    EntreeHash* prev = NULL;

    while (e) {
        if (strcmp(e->cle, nomClean) == 0) {
            // Retrait de la liste chaînée
            if (prev) {
                prev->suivant = e->suivant;
            } else {
                t->cases[idx] = e->suivant;
            }
            
            // Libération mémoire
            free(e);
            t->nbSchemes--;
            return 1;  // Suppression réussie
        }
        prev = e;
        e = e->suivant;
    }
    
    return 0;  // Schème non trouvé
}

// -----------------------------------------------------------------------------
// Affichage amélioré avec callback
// -----------------------------------------------------------------------------

void afficherSchemesCallback(const TableHash* t, AppendFunc append) {
    if (!append) {
        fprintf(stderr, "❌ Erreur : fonction callback NULL\n");
        return;
    }

    if (!t || t->nbSchemes == 0) {
        append("\n⚠️  Aucun schème morphologique disponible.\n\n");
        return;
    }

    char buf[300];
    
    append("\n╔═══════════════════════════════════════════════════════════╗\n");
    append("║       📐 SCHÈMES MORPHOLOGIQUES DISPONIBLES              ║\n");
    append("╚═══════════════════════════════════════════════════════════╝\n\n");

    int count = 0;
    for (int i = 0; i < TAILLE_TABLE; i++) {
        EntreeHash* e = t->cases[i];
        while (e) {
            count++;
            const char* regle = e->valeur.regleTransformation ? 
                               e->valeur.regleTransformation : 
                               "automatique";
            
            snprintf(buf, sizeof(buf), 
                    "  %3d. %-15s │ Pattern: %-20s │ Règle: %s\n",
                    count, e->valeur.nom, e->valeur.pattern, regle);
            append(buf);
            e = e->suivant;
        }
    }

    snprintf(buf, sizeof(buf), 
            "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n"
            "📊 Total : %d schème(s)\n\n", 
            t->nbSchemes);
    append(buf);
}

// Version printf simple (pour tests rapides)
void afficherSchemes(const TableHash* t) {
    if (!t) {
        printf("⚠️  Table de hachage NULL\n");
        return;
    }
    
    if (t->nbSchemes == 0) {
        printf("⚠️  Aucun schème disponible.\n\n");
        return;
    }

    printf("\n📐 Schèmes disponibles (%d) :\n", t->nbSchemes);
    printf("─────────────────────────────────────────────────\n");
    
    int count = 0;
    for (int i = 0; i < TAILLE_TABLE; i++) {
        EntreeHash* e = t->cases[i];
        while (e) {
            count++;
            printf("  %2d. %-15s | pattern: %s\n", 
                   count, e->valeur.nom, e->valeur.pattern);
            e = e->suivant;
        }
    }
    printf("─────────────────────────────────────────────────\n\n");
}

// -----------------------------------------------------------------------------
// Statistiques de la table de hachage
// -----------------------------------------------------------------------------

/**
 * Affiche les statistiques détaillées de la table de hachage
 * (taux de remplissage, collisions, distribution)
 */
void afficherStatistiquesHash(const TableHash* t, AppendFunc append) {
    if (!append || !t) return;

    int casesUtilisees = 0;
    int maxCollisions = 0;
    int totalCollisions = 0;
    
    // Analyse de la distribution
    for (int i = 0; i < TAILLE_TABLE; i++) {
        if (t->cases[i]) {
            casesUtilisees++;
            
            // Compter les éléments dans cette case
            int count = 0;
            EntreeHash* e = t->cases[i];
            while (e) {
                count++;
                e = e->suivant;
            }
            
            if (count > 1) {
                totalCollisions += (count - 1);
                if (count > maxCollisions) {
                    maxCollisions = count;
                }
            }
        }
    }

    char stats[2048];
    float tauxRemplissage = (float)casesUtilisees / TAILLE_TABLE * 100.0f;
    float facteurCharge = (float)t->nbSchemes / TAILLE_TABLE;
    
    snprintf(stats, sizeof(stats),
             "\n╔═══════════════════════════════════════════════════════════╗\n"
             "║      📊 STATISTIQUES TABLE DE HACHAGE                    ║\n"
             "╚═══════════════════════════════════════════════════════════╝\n\n"
             "  📐 Nombre de schèmes       : %d\n"
             "  📦 Taille de la table      : %d\n"
             "  ✅ Cases utilisées         : %d (%.1f%%)\n"
             "  ⚖️  Facteur de charge       : %.2f\n"
             "  💥 Collisions totales      : %d\n"
             "  📈 Collisions max (case)   : %d\n"
             "  🎯 Efficacité moyenne      : %.1f%%\n\n",
             t->nbSchemes,
             TAILLE_TABLE,
             casesUtilisees, tauxRemplissage,
             facteurCharge,
             totalCollisions,
             maxCollisions,
             t->nbSchemes > 0 ? (100.0f - (totalCollisions * 100.0f / t->nbSchemes)) : 100.0f);
    
    append(stats);
}

// -----------------------------------------------------------------------------
// Chargement depuis fichier
// -----------------------------------------------------------------------------

/**
 * Charge des schèmes depuis un fichier texte
 * Format attendu : nom|pattern|regle (une ligne par schème)
 * Exemple : فاعل|فاعل|auto
 */
int chargerSchemes(TableHash* t, const char* nomFichier) {
    if (!t || !nomFichier) return 0;

    FILE* f = fopen(nomFichier, "r");
    if (!f) {
        fprintf(stderr, "⚠️  Impossible d'ouvrir '%s' (fichier non trouvé)\n", nomFichier);
        return 0;
    }

    char ligne[512];
    int count = 0;

    while (fgets(ligne, sizeof(ligne), f)) {
        // Supprimer le retour à la ligne
        ligne[strcspn(ligne, "\r\n")] = '\0';
        
        // Ignorer les lignes vides et commentaires
        trimString(ligne);
        if (strlen(ligne) == 0 || ligne[0] == '#') continue;

        // Parser : nom|pattern|regle
        char nom[MAX_LEN] = {0};
        char pattern[MAX_LEN] = {0};
        char regle[MAX_LEN] = {0};

        char* token = strtok(ligne, "|");
        if (token) strncpy(nom, token, MAX_LEN - 1);
        
        token = strtok(NULL, "|");
        if (token) strncpy(pattern, token, MAX_LEN - 1);
        
        token = strtok(NULL, "|");
        if (token) strncpy(regle, token, MAX_LEN - 1);

        // Créer et insérer le schème
        if (strlen(nom) > 0 && strlen(pattern) > 0) {
            Scheme s;
            strncpy(s.nom, nom, MAX_LEN - 1);
            s.nom[MAX_LEN - 1] = '\0';
            strncpy(s.pattern, pattern, MAX_LEN - 1);
            s.pattern[MAX_LEN - 1] = '\0';
            
            // Règle optionnelle
            s.regleTransformation = NULL;  // Pour l'instant, on ne stocke pas dynamiquement
            
            insererScheme(t, s);
            count++;
        }
    }

    fclose(f);
    printf("✅ %d schème(s) chargé(s) depuis '%s'\n", count, nomFichier);
    return count;
}

// -----------------------------------------------------------------------------
// Sélection interactive
// -----------------------------------------------------------------------------

/**
 * Affiche les schèmes et permet à l'utilisateur d'en sélectionner un
 * Retourne le nom du schème sélectionné ou NULL
 */
const char* selectionnerSchemeParNumero(const TableHash* t) {
    if (!t || t->nbSchemes == 0) {
        printf("⚠️  Aucun schème disponible.\n");
        return NULL;
    }

    // Collecte des schèmes dans un tableau
    const char** schemes = malloc(t->nbSchemes * sizeof(const char*));
    if (!schemes) {
        printf("❌ Erreur allocation mémoire\n");
        return NULL;
    }

    int idx = 0;
    for (int i = 0; i < TAILLE_TABLE; i++) {
        EntreeHash* e = t->cases[i];
        while (e) {
            schemes[idx++] = e->cle;
            e = e->suivant;
        }
    }

    // Affichage
    printf("\n╔═══════════════════════════════════════════════════╗\n");
    printf("║        🎯 SÉLECTION D'UN SCHÈME                  ║\n");
    printf("╚═══════════════════════════════════════════════════╝\n\n");

    for (int i = 0; i < t->nbSchemes; i++) {
        printf("  %3d. %s\n", i + 1, schemes[i]);
    }

    printf("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("📝 Entrez le numéro (1 à %d) : ", t->nbSchemes);

    int choix;
    if (scanf("%d", &choix) != 1) {
        while (getchar() != '\n');
        printf("❌ Entrée invalide.\n");
        free(schemes);
        return NULL;
    }

    if (choix < 1 || choix > t->nbSchemes) {
        printf("❌ Numéro hors limites.\n");
        free(schemes);
        return NULL;
    }

    const char* selection = schemes[choix - 1];
    free(schemes);

    return selection;
}

// -----------------------------------------------------------------------------
// Libération complète
// -----------------------------------------------------------------------------

void libererTableHash(TableHash* t) {
    if (!t) return;

    for (int i = 0; i < TAILLE_TABLE; i++) {
        EntreeHash* e = t->cases[i];
        while (e) {
            EntreeHash* tmp = e;
            e = e->suivant;
            
            // Si regleTransformation est allouée dynamiquement, la libérer ici
            // if (tmp->valeur.regleTransformation) free(tmp->valeur.regleTransformation);
            
            free(tmp);
        }
    }
    free(t);
}

// -----------------------------------------------------------------------------
// Utilitaires
// -----------------------------------------------------------------------------

int compterSchemes(const TableHash* t) {
    return t ? t->nbSchemes : 0;
}

/**
 * Vérifie si un schème existe dans la table
 */
int schemeExiste(const TableHash* t, const char* nom) {
    return chercherScheme(t, nom) != NULL;
}