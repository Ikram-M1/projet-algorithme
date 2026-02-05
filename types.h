#ifndef TYPES_H
#define TYPES_H

// Taille max pour stocker un mot ou une racine (UTF-8 simplifié)
#define MAX_LEN 64

// -------- Dérivés d'une racine --------
typedef struct Derive {
    char mot[MAX_LEN];      // mot dérivé (ex: "مكتوب")
    int frequence;          // nombre de fois validé / généré
    struct Derive* suivant; // liste chaînée
} Derive;

// -------- Racine --------
typedef struct {
    char racine[MAX_LEN]; // ex: "كتب"
    Derive* derives;      // liste de mots dérivés validés
} Racine;

// -------- Noeud d'arbre (ABR, extensible en AVL) --------
typedef struct NoeudArbre {
    Racine data;
    struct NoeudArbre* gauche;
    struct NoeudArbre* droite;
    int hauteur;          // utile si tu fais AVL, sinon tu peux laisser à 0
} NoeudArbre;

// -------- Schème morphologique --------
// Pattern simplifié: chaîne avec '1','2','3' pour R1,R2,R3
// Exemple: "م1ت2و3ب" pour un schème imaginaire.
typedef struct {
    char nom[MAX_LEN];        // ex: "مفعول"
    char pattern[MAX_LEN];    // pattern interne, ex: "م1ك2و3ب" (à définir)
} Scheme;

// -------- Entrée de la table de hachage (chaînage) --------
typedef struct EntreeHash {
    char cle[MAX_LEN];        // nom du schème = clé
    Scheme valeur;            // schème complet
    struct EntreeHash* suivant;
} EntreeHash;

#define TAILLE_TABLE 101

typedef struct {
    EntreeHash* cases[TAILLE_TABLE];
} TableHash;

// -------- Contexte global de l'application --------
typedef struct {
    NoeudArbre* racines;  // racines dans l'arbre
    TableHash* schemes;   // table de hachage des schèmes
} Contexte;

#endif
