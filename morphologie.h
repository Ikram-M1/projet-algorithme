#ifndef MORPHOLOGIE_H
#define MORPHOLOGIE_H

#include "types.h"
#include "arbre.h"
#include "hash.h"

// -----------------------------------------------------------------------------
// Création de schèmes
// -----------------------------------------------------------------------------

/**
 * Crée un schème morphologique à partir d'un nom et d'un pattern.
 * Le pattern utilise 1, 2, 3 pour représenter les positions des consonnes radicales.
 * 
 * Exemples de patterns :
 * - "فاعل" avec pattern "1ا2ِ3" pour générer "كاتِب" depuis "كتب"
 * - "مفعول" avec pattern "م12و3" pour générer "مكتوب" depuis "كتب"
 * 
 * @param nom Nom du schème (ex: "فاعل", "مفعول")
 * @param pattern Modèle de transformation (ex: "1ا2ِ3", "م12و3")
 * @return Structure Scheme initialisée
 */
Scheme creerScheme(const char* nom, const char* pattern);

// -----------------------------------------------------------------------------
// Gestion des dérivés
// -----------------------------------------------------------------------------

/**
 * Ajoute un mot dérivé à une racine ou incrémente sa fréquence s'il existe déjà.
 * Le mot est automatiquement nettoyé (espaces supprimés).
 * 
 * @param r Pointeur vers la structure Racine
 * @param mot Le mot dérivé à ajouter
 */
void ajouterOuIncrementerDerive(Racine* r, const char* mot);

// -----------------------------------------------------------------------------
// Affichage des dérivés
// -----------------------------------------------------------------------------

/**
 * Affiche tous les dérivés d'une racine (version printf simple).
 * Format : mot (fréquence)
 * Note: Cette fonction est pour usage interne uniquement.
 * 
 * @param r Pointeur vers la structure Racine
 */
void afficherDerivesRacineSimple(const Racine* r);

// -----------------------------------------------------------------------------
// Génération morphologique (cœur du moteur)
// -----------------------------------------------------------------------------

/**
 * Génère un mot dérivé à partir d'une racine trilitère et d'un schème.
 * 
 * Processus :
 * 1. Valide que la racine est trilitère (3 lettres arabes = 6 octets UTF-8)
 * 2. Extrait les 3 consonnes radicales
 * 3. Applique le pattern du schème en substituant 1→r1, 2→r2, 3→r3
 * 
 * Exemple :
 * - Racine : "كتب" (k-t-b)
 * - Schème : {nom: "فاعل", pattern: "1ا2ِ3"}
 * - Résultat : "كاتِب" (katib = écrivain)
 * 
 * @param racine Racine trilitère arabe (doit être exactement 6 octets UTF-8)
 * @param sch Pointeur vers le schème à appliquer
 * @return Mot généré (à libérer avec free()), ou NULL en cas d'erreur
 * 
 * Complexité : O(m) où m = longueur du pattern
 */
char* genererMot(const char* racine, const Scheme* sch);

/**
 * Génère tous les dérivés possibles pour une racine donnée.
 * Applique tous les schèmes disponibles dans la table de hachage.
 * Les dérivés générés sont automatiquement ajoutés à la racine dans l'arbre.
 * 
 * @param arbre L'arbre AVL contenant les racines
 * @param t La table de hachage contenant les schèmes
 * @param racine La racine pour laquelle générer la famille morphologique
 */
void genererFamilleMorphologique(NoeudArbre* arbre, TableHash* t, const char* racine);

/**
 * Version callback de genererFamilleMorphologique (pour interface cohérente).
 * Génère la famille et affiche via callback.
 * 
 * @param arbre L'arbre AVL contenant les racines
 * @param t La table de hachage contenant les schèmes
 * @param racine La racine pour laquelle générer la famille
 * @param append Fonction callback pour l'affichage
 */
void genererFamilleMorphologiqueCallback(NoeudArbre* arbre, TableHash* t, 
                                         const char* racine, AppendFunc append);

// -----------------------------------------------------------------------------
// Validation morphologique
// -----------------------------------------------------------------------------

/**
 * Vérifie si un mot donné appartient morphologiquement à une racine.
 * 
 * Algorithme :
 * 1. Pour chaque schème disponible dans la table
 * 2. Génère le mot correspondant pour la racine donnée
 * 3. Compare avec le mot à valider
 * 4. Si match → retourne 1 et stocke le schème trouvé
 * 
 * Si validation réussie, le mot est automatiquement ajouté aux dérivés de la racine.
 * 
 * Exemple :
 * - Mot : "مكتوب"
 * - Racine : "كتب"
 * - Résultat : OUI (1) avec schème "مفعول"
 * 
 * @param arbre L'arbre AVL contenant les racines
 * @param t La table de hachage contenant les schèmes
 * @param mot Le mot à valider
 * @param racine La racine de référence
 * @param schTrouve Si non-NULL, stocke le pointeur vers le schème identifié
 * @return 1 si validation réussie (OUI), 0 sinon (NON)
 * 
 * Complexité : O(n × m) où n = nombre de schèmes, m = longueur du pattern
 */
int validerMotPourRacine(NoeudArbre* arbre, TableHash* t, const char* mot, 
                         const char* racine, Scheme** schTrouve);

// -----------------------------------------------------------------------------
// Extraction de racine (fonctionnalité avancée)
// -----------------------------------------------------------------------------

/**
 * Extrait la racine trilitère d'un mot dérivé (opération inverse).
 * ATTENTION : Fonction expérimentale, résultats non garantis.
 * 
 * Algorithme heuristique :
 * - Identifie les positions 1, 2, 3 dans le pattern
 * - Extrait les lettres correspondantes du mot
 * - Reconstruit la racine
 * 
 * @param mot Le mot dérivé à analyser
 * @param sch Le schème utilisé pour la dérivation
 * @return Racine extraite (à libérer avec free()), ou NULL en cas d'erreur
 * 
 * Note : Cette fonction nécessite une implémentation complète selon les besoins.
 */
char* extraireRacine(const char* mot, const Scheme* sch);

// -----------------------------------------------------------------------------
// Notes d'implémentation
// -----------------------------------------------------------------------------

/*
 * SYSTÈME MORPHOLOGIQUE ARABE :
 * ────────────────────────────────────────────────────────────────
 * La morphologie arabe repose sur le système racine-schème :
 * 
 * RACINE (جذر) :
 * - Ensemble de 3 consonnes (rarement 4) portant le sens de base
 * - Exemple : ك-ت-ب (k-t-b) = "écrire"
 * - UTF-8 : 3 lettres = 6 octets (chaque lettre arabe = 2 octets)
 * 
 * SCHÈME (وزن) :
 * - Modèle abstrait définissant la forme du mot
 * - Utilise 1, 2, 3 pour marquer les positions des consonnes radicales
 * - Inclut voyelles, préfixes, suffixes, infixes
 * 
 * EXEMPLES DE DÉRIVATION :
 * ────────────────────────────────────────────────────────────────
 * Racine : كتب (k-t-b)
 * 
 * Schème فاعل (acteur) :
 *   Pattern : "1ا2ِ3"
 *   Résultat : كاتِب (katib) = écrivain
 * 
 * Schème مفعول (passif) :
 *   Pattern : "م12و3"
 *   Résultat : مكتوب (maktub) = écrit
 * 
 * Schème افتعل (forme VIII) :
 *   Pattern : "ا1ت2َ3"
 *   Résultat : اكتتب (iktataba) = s'inscrire
 * 
 * ENCODAGE UTF-8 :
 * ────────────────────────────────────────────────────────────────
 * - Caractères arabes : U+0600 à U+06FF
 * - En UTF-8 : séquences de 2 octets (0xD8xx ou 0xD9xx)
 * - Voyelles courtes (tashkil) : diacritiques combinatoires
 * 
 * COMPLEXITÉS :
 * ────────────────────────────────────────────────────────────────
 * - Génération      : O(m) où m = longueur du pattern
 * - Validation      : O(n × m) où n = nombre de schèmes
 * - Famille complète: O(n × m) pour n schèmes
 * 
 * LIMITATIONS ACTUELLES :
 * ────────────────────────────────────────────────────────────────
 * - Racines trilitères uniquement (pas de quadrilitères)
 * - Patterns simples (1, 2, 3 + caractères arabes)
 * - Pas de gestion avancée des assimilations phonétiques
 * - Extraction de racine non implémentée complètement
 * 
 * AMÉLIORATIONS POSSIBLES :
 * ────────────────────────────────────────────────────────────────
 * - Support des racines quadrilitères (4 consonnes)
 * - Gestion des verbes faibles (voyelles longues dans la racine)
 * - Règles morphophonologiques (assimilation, élision)
 * - Base de données de racines avec sémantique
 * - Détection automatique du schème d'un mot
 */

#endif // MORPHOLOGIE_H