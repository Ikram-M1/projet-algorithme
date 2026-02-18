#ifndef HASH_H
#define HASH_H

#include "types.h"

// -----------------------------------------------------------------------------
// Création / Initialisation
// -----------------------------------------------------------------------------

/**
 * Crée et initialise une table de hachage vide.
 * Toutes les cases sont initialisées à NULL.
 * 
 * @return Pointeur vers la nouvelle table, ou NULL en cas d'erreur
 */
TableHash* creerTableHash();

// -----------------------------------------------------------------------------
// Opérations de base
// -----------------------------------------------------------------------------

/**
 * Insère ou met à jour un schème dans la table de hachage.
 * - Si le schème existe déjà (même nom) → mise à jour
 * - Sinon → insertion en tête de la liste chaînée
 * Le nom et le pattern sont automatiquement nettoyés (espaces supprimés).
 * 
 * @param t La table de hachage
 * @param s Le schème à insérer (copié par valeur)
 * 
 * Complexité : O(1) en moyenne, O(n) au pire cas (avec collisions)
 */
void insererScheme(TableHash* t, Scheme s);

/**
 * Recherche un schème par son nom dans la table.
 * Le nom de recherche est automatiquement nettoyé avant comparaison.
 * 
 * @param t La table de hachage
 * @param nom Le nom du schème à rechercher
 * @return Pointeur vers le schème trouvé, ou NULL si non trouvé
 * 
 * Complexité : O(1) en moyenne, O(n) au pire cas
 */
Scheme* chercherScheme(TableHash* t, const char* nom);

/**
 * Supprime un schème de la table par son nom.
 * 
 * @param t La table de hachage
 * @param nom Le nom du schème à supprimer
 * @return 1 si suppression réussie, 0 si schème non trouvé
 * 
 * Complexité : O(1) en moyenne, O(n) au pire cas
 */
int supprimerScheme(TableHash* t, const char* nom);

/**
 * Vérifie si un schème existe dans la table.
 * 
 * @param t La table de hachage
 * @param nom Le nom du schème à vérifier
 * @return 1 si le schème existe, 0 sinon
 */
int schemeExiste(const TableHash* t, const char* nom);

// -----------------------------------------------------------------------------
// Affichage
// -----------------------------------------------------------------------------

/**
 * Affiche tous les schèmes de la table (version printf simple).
 * Utile pour débogage rapide.
 * 
 * @param t La table de hachage à afficher
 */
void afficherSchemes(const TableHash* t);

/**
 * Affiche tous les schèmes via une fonction callback (version améliorée).
 * Format : numéro | nom | pattern | règle
 * Inclut bordures et emojis pour meilleure lisibilité.
 * 
 * @param t La table de hachage
 * @param append Fonction callback pour ajouter du texte à l'affichage
 */
void afficherSchemesCallback(const TableHash* t, AppendFunc append);

// -----------------------------------------------------------------------------
// Statistiques et analyse
// -----------------------------------------------------------------------------

/**
 * Affiche des statistiques détaillées sur la table de hachage.
 * Inclut : nombre de schèmes, taux de remplissage, facteur de charge,
 * nombre de collisions, efficacité moyenne.
 * 
 * @param t La table de hachage
 * @param append Fonction callback pour ajouter du texte à l'affichage
 */
void afficherStatistiquesHash(const TableHash* t, AppendFunc append);

/**
 * Compte le nombre total de schèmes dans la table.
 * Utilise le compteur interne pour O(1).
 * 
 * @param t La table de hachage
 * @return Nombre de schèmes stockés
 */
int compterSchemes(const TableHash* t);

// -----------------------------------------------------------------------------
// Chargement et sauvegarde
// -----------------------------------------------------------------------------

/**
 * Charge des schèmes depuis un fichier texte.
 * Format attendu : nom|pattern|regle (une ligne par schème)
 * Les lignes vides et commençant par '#' sont ignorées.
 * 
 * Exemple de fichier schemes.txt :
 * # Schèmes morphologiques arabes
 * فاعل|فاعل|auto
 * مفعول|مفعول|auto
 * افتعل|افتعل|auto
 * 
 * @param t La table de hachage où charger les schèmes
 * @param nomFichier Chemin du fichier à charger
 * @return Nombre de schèmes chargés, 0 en cas d'erreur
 */
int chargerSchemes(TableHash* t, const char* nomFichier);

// -----------------------------------------------------------------------------
// Sélection interactive
// -----------------------------------------------------------------------------

/**
 * Interface interactive : affiche la liste numérotée des schèmes,
 * demande à l'utilisateur de choisir un numéro, et retourne le nom du schème.
 * 
 * @param t La table de hachage
 * @return Nom du schème sélectionné, ou NULL en cas d'erreur/annulation
 */
const char* selectionnerSchemeParNumero(const TableHash* t);

// -----------------------------------------------------------------------------
// Libération mémoire
// -----------------------------------------------------------------------------

/**
 * Libère complètement la table de hachage et toutes ses entrées.
 * Parcourt toutes les cases et libère toutes les listes chaînées.
 * Doit être appelé à la fin du programme pour éviter les fuites mémoire.
 * 
 * @param t La table de hachage à libérer
 */
void libererTableHash(TableHash* t);

// -----------------------------------------------------------------------------
// Notes d'implémentation
// -----------------------------------------------------------------------------

/*
 * FONCTION DE HACHAGE :
 * - Algorithme DJB2 (Daniel J. Bernstein) : h = h * 33 + c
 * - Excellente distribution pour chaînes courtes et UTF-8
 * - Constante magique : 5381
 * 
 * GESTION DES COLLISIONS :
 * - Chaînage avec listes chaînées (insertion en tête)
 * - Complexité moyenne O(1) si facteur de charge < 1
 * 
 * COMPLEXITÉS :
 * - Insertion    : O(1) en moyenne
 * - Recherche    : O(1) en moyenne
 * - Suppression  : O(1) en moyenne
 * - Affichage    : O(n) où n = nombre de schèmes
 * 
 * TAILLE DE LA TABLE :
 * - Définie dans types.h comme TAILLE_TABLE
 * - Recommandé : nombre premier (ex: 101, 151, 211)
 * - Facteur de charge optimal : < 0.75
 * 
 * ENCODAGE :
 * - Support complet UTF-8 pour caractères arabes
 * - Nettoyage automatique des espaces
 * - Validation des noms de schèmes
 * 
 * FORMAT DES SCHÈMES :
 * - nom : identifiant du schème (ex: "فاعل")
 * - pattern : représentation abstraite (ex: "ف_ع_ل")
 * - regleTransformation : règle optionnelle (ex: "auto", "custom")
 */

#endif // HASH_H