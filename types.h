#ifndef TYPES_H
#define TYPES_H

// -----------------------------------------------------------------------------
// Constantes globales
// -----------------------------------------------------------------------------

/**
 * Taille maximale pour stocker un mot, une racine ou un pattern en UTF-8.
 * 64 octets permettent de stocker environ 32 lettres arabes (2 octets/lettre).
 */
#define MAX_LEN 64

/**
 * Taille de la table de hachage (nombre premier pour meilleure distribution).
 * 101 est un bon compromis entre mémoire et performance pour ~50-100 schèmes.
 * Augmenter à 211 ou 307 si vous avez plus de schèmes.
 */
#define TAILLE_TABLE 101

// -----------------------------------------------------------------------------
// Type pour les fonctions callback d'affichage
// -----------------------------------------------------------------------------

/**
 * Type de fonction callback utilisé pour l'affichage modulaire.
 * Permet de rediriger l'affichage vers printf, un buffer, un fichier, ou une GUI.
 * 
 * Exemple d'utilisation :
 *   void myAppend(const char* str) { printf("%s", str); }
 *   afficherRacinesCallback(arbre, myAppend);
 */
typedef void (*AppendFunc)(const char*);

// -----------------------------------------------------------------------------
// Structures pour les dérivés morphologiques
// -----------------------------------------------------------------------------

/**
 * Structure représentant un mot dérivé d'une racine.
 * Stocké dans une liste chaînée pour chaque racine.
 */
typedef struct Derive {
    char mot[MAX_LEN];      /**< Mot dérivé (ex: "مكتوب", "كاتب") */
    int frequence;          /**< Nombre de fois que ce dérivé a été généré/validé */
    struct Derive* suivant; /**< Pointeur vers le prochain dérivé (liste chaînée) */
} Derive;

// -----------------------------------------------------------------------------
// Structure pour les racines arabes
// -----------------------------------------------------------------------------

/**
 * Structure représentant une racine trilitère arabe.
 * Contient la racine elle-même et la liste de ses dérivés validés.
 */
typedef struct {
    char racine[MAX_LEN];   /**< Racine trilitère (ex: "كتب", "قرأ") */
    Derive* derives;        /**< Liste chaînée des mots dérivés validés */
    int nbDerives;          /**< Compteur pour optimiser (évite recomptage) */
} Racine;

// -----------------------------------------------------------------------------
// Structure pour l'arbre AVL des racines
// -----------------------------------------------------------------------------

/**
 * Nœud de l'arbre AVL contenant une racine.
 * L'arbre est auto-équilibré pour garantir O(log n) en recherche/insertion.
 */
typedef struct NoeudArbre {
    Racine data;                /**< Données de la racine stockée */
    struct NoeudArbre* gauche;  /**< Sous-arbre gauche */
    struct NoeudArbre* droite;  /**< Sous-arbre droit */
    int hauteur;                /**< Hauteur du nœud (pour équilibrage AVL) */
} NoeudArbre;

// -----------------------------------------------------------------------------
// Structure pour les schèmes morphologiques
// -----------------------------------------------------------------------------

/**
 * Structure représentant un schème morphologique (وزن - wazn).
 * 
 * Le pattern utilise une notation simplifiée :
 * - '1' = première consonne radicale (R1)
 * - '2' = deuxième consonne radicale (R2)
 * - '3' = troisième consonne radicale (R3)
 * - Autres caractères = lettres fixes du schème
 * 
 * Exemples :
 * - فاعل : pattern "1ا23" (R1 + ا + R2 + R3)
 * - مفعول : pattern "م12و3" (م + R1 + R2 + و + R3)
 * - استفعل : pattern "است123" (است + R1 + R2 + R3)
 */
typedef struct {
    char nom[MAX_LEN];              /**< Nom du schème (ex: "فاعل", "مفعول") */
    char pattern[MAX_LEN];          /**< Pattern de transformation (ex: "1ا23", "م12و3") */
    char* regleTransformation;      /**< Règle optionnelle (pour schèmes complexes) */
} Scheme;

// -----------------------------------------------------------------------------
// Structure pour la table de hachage (chaînage)
// -----------------------------------------------------------------------------

/**
 * Entrée de la table de hachage pour stocker un schème.
 * Utilise le chaînage pour résoudre les collisions.
 */
typedef struct EntreeHash {
    char cle[MAX_LEN];              /**< Clé = nom du schème */
    Scheme valeur;                  /**< Valeur = structure Scheme complète */
    struct EntreeHash* suivant;     /**< Pointeur vers entrée suivante (collision) */
} EntreeHash;

/**
 * Table de hachage pour stocker tous les schèmes morphologiques.
 * Utilise la fonction de hachage DJB2 pour distribuer les schèmes.
 */
typedef struct {
    EntreeHash* cases[TAILLE_TABLE]; /**< Tableau de pointeurs (chaînage) */
    int nbSchemes;                   /**< Compteur total de schèmes (optimisation) */
} TableHash;

// -----------------------------------------------------------------------------
// Contexte global de l'application
// -----------------------------------------------------------------------------

/**
 * Structure globale contenant l'état de l'application.
 * Permet de passer facilement les données entre les fonctions.
 */
typedef struct {
    NoeudArbre* racines;  /**< Arbre AVL contenant toutes les racines */
    TableHash* schemes;   /**< Table de hachage contenant tous les schèmes */
} Contexte;

// -----------------------------------------------------------------------------
// Documentation des structures de données
// -----------------------------------------------------------------------------

/*
 * ORGANISATION MÉMOIRE :
 * ══════════════════════════════════════════════════════════════════
 * 
 * 1. ARBRE AVL DES RACINES
 *    ┌─────────────┐
 *    │   كتب      │ hauteur=2
 *    │ nbDerives=3 │
 *    │  derives ───┼──→ [مكتوب]──→[كاتب]──→[كتاب]──→NULL
 *    └──┬───────┬──┘
 *       │       │
 *    [قرأ]    [علم]
 * 
 * 2. TABLE DE HACHAGE DES SCHÈMES
 *    Index  Chaîne
 *    ─────  ──────────────────────────
 *      0 →  NULL
 *      1 →  NULL
 *      5 →  [فاعل]──→[فعيل]──→NULL
 *     23 →  [مفعول]──→NULL
 *     ...
 *    100 →  NULL
 * 
 * 3. LISTE CHAÎNÉE DES DÉRIVÉS
 *    [مكتوب, freq=2] → [كاتب, freq=1] → [كتاب, freq=1] → NULL
 * 
 * COMPLEXITÉS THÉORIQUES :
 * ══════════════════════════════════════════════════════════════════
 * 
 * Arbre AVL :
 * - Insertion       : O(log n)
 * - Recherche       : O(log n)
 * - Suppression     : O(log n)
 * - Parcours        : O(n)
 * - Hauteur max     : 1.44 × log₂(n+2)
 * 
 * Table de hachage :
 * - Insertion       : O(1) en moyenne, O(n) pire cas
 * - Recherche       : O(1) en moyenne, O(n) pire cas
 * - Suppression     : O(1) en moyenne, O(n) pire cas
 * - Facteur charge  : n/m (optimal si < 0.75)
 * 
 * Listes chaînées (dérivés) :
 * - Insertion tête  : O(1)
 * - Recherche       : O(k) où k = nombre de dérivés
 * - Parcours        : O(k)
 * 
 * UTILISATION MÉMOIRE ESTIMÉE :
 * ══════════════════════════════════════════════════════════════════
 * 
 * Pour 100 racines avec 5 dérivés chacune, 10 schèmes :
 * 
 * - NoeudArbre   : 100 × (~200 octets) = 20 KB
 * - Derive       : 500 × (~80 octets)  = 40 KB
 * - TableHash    : 101 × 8 octets      = 808 octets
 * - EntreeHash   : 10 × (~150 octets)  = 1.5 KB
 * 
 * Total estimé : ~62 KB (très raisonnable)
 * 
 * ENCODAGE UTF-8 :
 * ══════════════════════════════════════════════════════════════════
 * 
 * - Caractères arabes : 2 octets par lettre
 * - MAX_LEN = 64 → ~32 lettres arabes maximum
 * - Racine trilitère : 6 octets (3 lettres × 2)
 * - Mot dérivé moyen : 10-20 octets (5-10 lettres)
 * 
 * LIMITES ACTUELLES :
 * ══════════════════════════════════════════════════════════════════
 * 
 * - MAX_LEN = 64 : suffisant pour mots courts/moyens
 * - TAILLE_TABLE = 101 : optimal pour ~75 schèmes
 * - Racines trilitères uniquement (pas de quadrilitères)
 * - Pas de gestion des hamza, shadda complexes
 * 
 * EXTENSIONS POSSIBLES :
 * ══════════════════════════════════════════════════════════════════
 * 
 * - Augmenter MAX_LEN à 128 pour mots composés
 * - Redimensionnement dynamique de la table de hachage
 * - Support des racines quadrilitères
 * - Métadonnées sémantiques (sens de la racine)
 * - Timestamps pour les dérivés (date d'ajout)
 */

#endif // TYPES_H