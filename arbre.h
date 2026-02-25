#ifndef ARBRE_H
#define ARBRE_H

#include "types.h"  // Contient : NoeudArbre, Racine, Derive, MAX_LEN, AppendFunc

// -----------------------------------------------------------------------------
// Création et gestion de base des nœuds AVL
// -----------------------------------------------------------------------------

/**
 * Crée un nouveau nœud AVL contenant une racine arabe.
 * Initialise hauteur = 1, gauche/droite = NULL, derives = NULL, nbDerives = 0.
 * 
 * @param racineStr La chaîne représentant la racine arabe (sera nettoyée)
 * @return Pointeur vers le nouveau nœud, ou NULL en cas d'erreur
 */
NoeudArbre* creerNoeud(const char* racineStr);

// -----------------------------------------------------------------------------
// Opérations sur l'arbre AVL
// -----------------------------------------------------------------------------

/**
 * Insère une nouvelle racine dans l'arbre AVL (équilibré automatiquement).
 * Si la racine existe déjà, ne fait rien (pas de doublons).
 * La chaîne est automatiquement nettoyée (espaces supprimés).
 * 
 * @param racine La racine actuelle de l'arbre (peut être NULL)
 * @param nouvelleRacine La racine arabe à insérer
 * @return La nouvelle racine de l'arbre après insertion et rééquilibrage
 * 
 * Complexité : O(log n) en moyenne, O(n) au pire cas
 */
NoeudArbre* insererRacine(NoeudArbre* racine, const char* nouvelleRacine);

/**
 * Recherche une racine dans l'arbre AVL.
 * La clé de recherche est automatiquement nettoyée avant comparaison.
 * 
 * @param racine La racine de l'arbre où effectuer la recherche
 * @param cle La racine arabe à rechercher
 * @return Pointeur vers le nœud si trouvé, NULL sinon
 * 
 * Complexité : O(log n) grâce à l'équilibrage AVL
 */
NoeudArbre* rechercherRacine(NoeudArbre* racine, const char* cle);

/**
 * Charge les racines depuis un fichier texte (une racine par ligne).
 * Déclarée ici pour rendre la fonction accessible aux modules GUI.
 */
void chargerRacinesDepuisFichier(NoeudArbre** arbre, const char* nomFichier);

// -----------------------------------------------------------------------------
// Fonctions utilitaires AVL (publiques pour inspection)
// -----------------------------------------------------------------------------

/**
 * Retourne la hauteur d'un nœud (0 si NULL).
 * Utile pour vérifier l'équilibre de l'arbre.
 */
int hauteur(NoeudArbre* n);

/**
 * Calcule le facteur d'équilibre d'un nœud.
 * Facteur = hauteur(gauche) - hauteur(droite)
 * Valeurs acceptables : -1, 0, +1 (arbre équilibré)
 */
int equilibre(NoeudArbre* n);

/**
 * Retourne le maximum de deux entiers.
 */
int max(int a, int b);

// -----------------------------------------------------------------------------
// Affichage et sélection
// -----------------------------------------------------------------------------

/**
 * Affiche toutes les racines triées alphabétiquement via un callback.
 * Utilise une collecte + qsort pour garantir un ordre correct.
 * L'affichage inclut des bordures Unicode et des emojis pour meilleure lisibilité.
 * 
 * Note : l'ordre est basé sur strcmp (approximation Unicode, pas collation arabe stricte).
 * 
 * @param racine La racine de l'arbre à afficher
 * @param append Fonction callback pour ajouter du texte à l'affichage
 */
void afficherRacinesCallback(NoeudArbre* racine, AppendFunc append);

/**
 * Interface interactive : affiche la liste numérotée des racines triées,
 * demande à l'utilisateur de choisir un numéro, et retourne la chaîne de la racine sélectionnée.
 * 
 * @param racine La racine de l'arbre pour la sélection
 * @return Chaîne de la racine sélectionnée, ou NULL en cas d'erreur/annulation
 */
const char* selectionnerRacineParNumero(NoeudArbre* racine);

// -----------------------------------------------------------------------------
// Gestion des dérivés associés à une racine
// -----------------------------------------------------------------------------

/**
 * Ajoute un mot dérivé à la liste chaînée d'une racine.
 * - Si le mot existe déjà → incrémente sa fréquence
 * - Sinon → ajoute en tête de liste et incrémente nbDerives
 * Le mot est automatiquement nettoyé (espaces supprimés).
 * 
 * @param noeud Le nœud de la racine où ajouter le dérivé
 * @param motDerive Le mot dérivé à ajouter
 */
void ajouterDerive(NoeudArbre* noeud, const char* motDerive);

/**
 * Affiche la liste des dérivés validés pour une racine donnée (via callback).
 * Affiche : racine + nombre total + mot + fréquence pour chaque dérivé.
 * Format amélioré avec bordures et emojis.
 * 
 * @param noeud Le nœud contenant les dérivés à afficher
 * @param append Fonction callback pour ajouter du texte à l'affichage
 */
void afficherDerivesRacine(NoeudArbre* noeud, AppendFunc append);

// -----------------------------------------------------------------------------
// Statistiques et compteurs
// -----------------------------------------------------------------------------

/**
 * Compte le nombre total de dérivés stockés dans tout l'arbre.
 * Parcourt récursivement tous les nœuds et additionne leurs nbDerives.
 * 
 * @param racine La racine de l'arbre
 * @return Nombre total de dérivés dans l'arbre
 * 
 * Complexité : O(n) où n = nombre de nœuds
 */
int compterDerivesTotaux(NoeudArbre* racine);

/**
 * Affiche des statistiques complètes sur l'arbre (via callback).
 * Inclut : nombre de racines, nombre total de dérivés, hauteur de l'arbre,
 * moyenne de dérivés par racine, facteur d'équilibre de la racine.
 * 
 * @param racine La racine de l'arbre
 * @param append Fonction callback pour ajouter du texte à l'affichage
 */
void afficherStatistiquesArbre(NoeudArbre* racine, AppendFunc append);

// -----------------------------------------------------------------------------
// Libération mémoire
// -----------------------------------------------------------------------------

/**
 * Libère la mémoire d'une liste chaînée de dérivés.
 * Parcourt la liste et libère chaque nœud.
 * 
 * @param d Pointeur vers le premier dérivé de la liste
 */
void libererDerives(Derive* d);

/**
 * Libère récursivement tout l'arbre AVL + toutes les listes de dérivés associées.
 * Doit être appelé à la fin du programme pour éviter les fuites mémoire.
 * 
 * @param racine La racine de l'arbre à libérer
 * 
 * Ordre de libération : gauche → droite → dérivés → nœud actuel
 */
void libererArbre(NoeudArbre* racine);

// -----------------------------------------------------------------------------
// Fonctions internes (rotations AVL - déclarées pour cohérence)
// -----------------------------------------------------------------------------

/**
 * Effectue une rotation droite sur un sous-arbre.
 * Utilisée pour rééquilibrer l'arbre AVL (cas Left-Left ou Left-Right).
 * 
 * @param y Le nœud déséquilibré
 * @return La nouvelle racine du sous-arbre après rotation
 */
NoeudArbre* rotationDroite(NoeudArbre* y);

/**
 * Effectue une rotation gauche sur un sous-arbre.
 * Utilisée pour rééquilibrer l'arbre AVL (cas Right-Right ou Right-Left).
 * 
 * @param x Le nœud déséquilibré
 * @return La nouvelle racine du sous-arbre après rotation
 */
NoeudArbre* rotationGauche(NoeudArbre* x);

// -----------------------------------------------------------------------------
// Notes d'implémentation
// -----------------------------------------------------------------------------

/*
 * COMPLEXITÉS ALGORITHMIQUES :
 * - Insertion       : O(log n) amortie
 * - Recherche       : O(log n) 
 * - Affichage trié  : O(n log n) (collecte O(n) + tri O(n log n))
 * - Statistiques    : O(n)
 * - Libération      : O(n)
 * 
 * GARANTIES AVL :
 * - Hauteur ≤ 1.44 × log₂(n+2) - 0.328
 * - Facteur d'équilibre ∈ {-1, 0, +1} pour tous les nœuds
 * 
 * ENCODAGE :
 * - Support UTF-8 pour les caractères arabes
 * - Nettoyage automatique des espaces avant/après
 * - Validation basique des entrées
 * 
 * GESTION MÉMOIRE :
 * - Allocation dynamique pour tous les nœuds
 * - Listes chaînées pour les dérivés (insertion O(1))
 * - Libération complète via libererArbre()
 */

#endif // ARBRE_H