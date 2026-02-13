#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#include "types.h"
#include "arbre.h"
#include "hash.h"
#include "morphologie.h"

// Déclaration de la fonction GUI (si disponible dans gui.c)
extern int lancer_gui(int argc, char *argv[]) __attribute__((weak));

// Contexte global de l'application
Contexte ctx;

// Fonction publique pour charger racines (accessible par gui.c)
void chargerRacinesDepuisFichier(NoeudArbre** arbre, const char* nomFichier);

// Buffer pour l'affichage avec callback
static char displayBuffer[50000];

// Fonction callback pour AppendFunc
static void appendToBuffer(const char* str) {
    if (strlen(displayBuffer) + strlen(str) < sizeof(displayBuffer) - 1) {
        strcat(displayBuffer, str);
    }
}

// Fonction pour afficher le buffer et le vider
static void flushBuffer() {
    printf("%s", displayBuffer);
    displayBuffer[0] = '\0';
}

// -----------------------------------------------------------------------------
// Fonctions utilitaires
// -----------------------------------------------------------------------------

/**
 * Vide le buffer stdin pour éviter les problèmes de saisie
 */
static void viderBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

/**
 * Lit une ligne sécurisée depuis stdin
 */
static int lireLigne(char* buffer, int taille) {
    if (!fgets(buffer, taille, stdin)) {
        return 0;
    }
    buffer[strcspn(buffer, "\r\n")] = '\0';
    return 1;
}

/**
 * Affiche un titre avec bordure
 */
static void afficherTitre(const char* titre) {
    printf("\n╔═══════════════════════════════════════════════════════════╗\n");
    printf("║  %-55s  ║\n", titre);
    printf("╚═══════════════════════════════════════════════════════════╝\n\n");
}

/**
 * Affiche un séparateur
 */
static void afficherSeparateur() {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
}

// -----------------------------------------------------------------------------
// Chargement et initialisation
// -----------------------------------------------------------------------------

/**
 * Charge les racines depuis un fichier texte
 * Format : une racine par ligne
 */
void chargerRacinesDepuisFichier(NoeudArbre** arbre, const char* nomFichier) {
    FILE* f = fopen(nomFichier, "r");
    if (!f) {
        printf("⚠️  Impossible d'ouvrir '%s'\n", nomFichier);
        printf("    Le fichier n'existe peut-être pas encore.\n");
        printf("    Vous pouvez créer un fichier %s avec une racine par ligne.\n\n", nomFichier);
        return;
    }

    char ligne[MAX_LEN];
    int count = 0;
    int erreurs = 0;

    printf("📂 Chargement des racines depuis '%s'...\n", nomFichier);

    while (fgets(ligne, sizeof(ligne), f)) {
        // Supprime le retour à la ligne
        ligne[strcspn(ligne, "\r\n")] = '\0';
        
        // Ignore les lignes vides et les commentaires
        if (strlen(ligne) > 0 && ligne[0] != '#') {
            NoeudArbre* ancien = *arbre;
            *arbre = insererRacine(*arbre, ligne);
            if (*arbre != ancien || ancien == NULL) {
                count++;
            } else {
                erreurs++;
            }
        }
    }
    fclose(f);

    printf("✅ %d racine(s) chargée(s)", count);
    if (erreurs > 0) {
        printf(" (%d doublon(s) ignoré(s))", erreurs);
    }
    printf("\n\n");
}

/**
 * Initialise les schèmes de base pour le système
 */
void initialiserSchemesDeBase(TableHash* t) {
    if (!t) return;

    printf("🔧 Initialisation des schèmes de base...\n");

    // Schèmes classiques de la morphologie arabe
    Scheme schemes[] = {
        {"فاعل",    "1ا23",     NULL},    // Agent/acteur
        {"مفعول",   "م12و3",    NULL},    // Participe passif
        {"افتعل",   "ا1ت23",    NULL},    // Forme VIII
        {"تفعيل",   "ت12ي3",    NULL},    // Nom d'action forme II
        {"مفعال",   "م12ا3",    NULL},    // Nom d'instrument
        {"فعيل",    "123",      NULL},    // Adjectif intensif
        {"فعّال",   "12ّا3",    NULL},    // Métier/profession
        {"استفعل",  "است123",   NULL}     // Forme X
    };

    int nb = sizeof(schemes) / sizeof(Scheme);
    for (int i = 0; i < nb; i++) {
        insererScheme(t, schemes[i]);
    }

    printf("✅ %d schème(s) initialisé(s)\n\n", nb);
}

// -----------------------------------------------------------------------------
// Menu principal
// -----------------------------------------------------------------------------

void afficherMenu() {
    printf("\n");
    printf("╔═══════════════════════════════════════════════════════════╗\n");
    printf("║         🌙 MOTEUR MORPHOLOGIQUE ARABE - القاموس         ║\n");
    printf("╚═══════════════════════════════════════════════════════════╝\n");
    printf("\n");
    printf("  📚 GESTION DES RACINES\n");
    printf("    1️⃣  Charger racines depuis fichier\n");
    printf("    2️⃣  Afficher toutes les racines\n");
    printf("    3️⃣  Ajouter une nouvelle racine\n");
    printf("    4️⃣  Rechercher une racine\n");
    printf("\n");
    printf("  🔄 GÉNÉRATION MORPHOLOGIQUE\n");
    printf("    5️⃣  Générer les dérivés d'une racine\n");
    printf("    6️⃣  Afficher les dérivés existants\n");
    printf("\n");
    printf("  ✅ VALIDATION\n");
    printf("    7️⃣  Vérifier si un mot appartient à une racine\n");
    printf("\n");
    printf("  📐 GESTION DES SCHÈMES\n");
    printf("    8️⃣  Afficher les schèmes disponibles\n");
    printf("    9️⃣  Charger schèmes depuis fichier\n");
    printf("    🔟 Ajouter un schème\n");
    printf("    1️⃣1️⃣ Modifier un schème\n");
    printf("    1️⃣2️⃣ Supprimer un schème\n");
    printf("\n");
    printf("  📊 STATISTIQUES\n");
    printf("    1️⃣3️⃣ Afficher statistiques de l'arbre\n");
    printf("    1️⃣4️⃣ Afficher statistiques de la table de hachage\n");
    printf("\n");
    printf("  💾 SAUVEGARDE\n");
    printf("    1️⃣5️⃣ Sauvegarder les racines\n");
    printf("\n");
    printf("  ❌ QUITTER\n");
    printf("    0️⃣  Quitter l'application\n");
    printf("\n");
    afficherSeparateur();
    printf("👉 Votre choix : ");
}

// -----------------------------------------------------------------------------
// Fonctions du menu
// -----------------------------------------------------------------------------

void menuChargerRacines() {
    afficherTitre("📂 CHARGEMENT DES RACINES");
    printf("Fichier par défaut : racines.txt\n");
    printf("Appuyez sur Entrée pour utiliser le défaut, ou tapez un autre nom : ");
    
    char nomFichier[256];
    if (!lireLigne(nomFichier, sizeof(nomFichier)) || strlen(nomFichier) == 0) {
        strcpy(nomFichier, "racines.txt");
    }
    
    chargerRacinesDepuisFichier(&ctx.racines, nomFichier);
}

void menuAfficherRacines() {
    afficherTitre("📚 LISTE DES RACINES");
    displayBuffer[0] = '\0';
    afficherRacinesCallback(ctx.racines, appendToBuffer);
    flushBuffer();
}

void menuAjouterRacine() {
    afficherTitre("➕ AJOUTER UNE RACINE");
    printf("Entrez la racine trilitère (ex: كتب) : ");
    
    char buffer[MAX_LEN];
    if (!lireLigne(buffer, sizeof(buffer))) {
        printf("❌ Erreur de lecture\n");
        return;
    }
    
    NoeudArbre* avant = ctx.racines;
    ctx.racines = insererRacine(ctx.racines, buffer);
    
    if (ctx.racines != avant || avant == NULL) {
        printf("✅ Racine '%s' ajoutée avec succès\n\n", buffer);
    } else {
        printf("⚠️  La racine '%s' existe déjà\n\n", buffer);
    }
}

void menuRechercherRacine() {
    afficherTitre("🔍 RECHERCHER UNE RACINE");
    printf("Entrez la racine à rechercher : ");
    
    char buffer[MAX_LEN];
    if (!lireLigne(buffer, sizeof(buffer))) {
        printf("❌ Erreur de lecture\n");
        return;
    }
    
    NoeudArbre* n = rechercherRacine(ctx.racines, buffer);
    if (n) {
        printf("✅ Racine '%s' trouvée !\n", buffer);
        printf("   Nombre de dérivés : %d\n\n", n->data.nbDerives);
    } else {
        printf("❌ Racine '%s' non trouvée\n\n", buffer);
    }
}

void menuGenererDerives() {
    afficherTitre("🔄 GÉNÉRATION DE DÉRIVÉS");
    
    const char* racineChoisie = selectionnerRacineParNumero(ctx.racines);
    if (racineChoisie) {
        displayBuffer[0] = '\0';
        genererFamilleMorphologiqueCallback(ctx.racines, ctx.schemes, 
                                           racineChoisie, appendToBuffer);
        flushBuffer();
    } else {
        printf("❌ Aucune racine sélectionnée\n\n");
    }
}

void menuAfficherDerives() {
    afficherTitre("📖 AFFICHER LES DÉRIVÉS");
    
    const char* racineChoisie = selectionnerRacineParNumero(ctx.racines);
    if (!racineChoisie) {
        printf("❌ Aucune racine sélectionnée\n\n");
        return;
    }
    
    NoeudArbre* n = rechercherRacine(ctx.racines, racineChoisie);
    if (n) {
        displayBuffer[0] = '\0';
        afficherDerivesRacine(n, appendToBuffer);
        flushBuffer();
    } else {
        printf("❌ Racine introuvable\n\n");
    }
}

void menuValiderMot() {
    afficherTitre("✅ VALIDATION MORPHOLOGIQUE");
    
    printf("Entrez le mot à valider : ");
    char mot[MAX_LEN];
    if (!lireLigne(mot, sizeof(mot))) {
        printf("❌ Erreur de lecture\n");
        return;
    }
    
    printf("Entrez la racine de référence : ");
    char racine[MAX_LEN];
    if (!lireLigne(racine, sizeof(racine))) {
        printf("❌ Erreur de lecture\n");
        return;
    }
    
    Scheme* schTrouve = NULL;
    printf("\n🔍 Validation en cours...\n\n");
    
    if (validerMotPourRacine(ctx.racines, ctx.schemes, mot, racine, &schTrouve)) {
        printf("✅ RÉSULTAT : OUI\n\n");
        printf("   Le mot '%s' appartient à la racine '%s'\n", mot, racine);
        if (schTrouve) {
            printf("   Schème identifié : %s (pattern: %s)\n", 
                   schTrouve->nom, schTrouve->pattern);
        }
        printf("\n");
    } else {
        printf("❌ RÉSULTAT : NON\n\n");
        printf("   Le mot '%s' n'appartient PAS à la racine '%s'\n\n", mot, racine);
    }
}

void menuAfficherSchemes() {
    afficherTitre("📐 SCHÈMES MORPHOLOGIQUES");
    displayBuffer[0] = '\0';
    afficherSchemesCallback(ctx.schemes, appendToBuffer);
    flushBuffer();
}

void menuChargerSchemes() {
    afficherTitre("📂 CHARGEMENT DES SCHÈMES");
    printf("Fichier par défaut : schemes.txt\n");
    printf("Appuyez sur Entrée pour utiliser le défaut, ou tapez un autre nom : ");
    
    char nomFichier[256];
    if (!lireLigne(nomFichier, sizeof(nomFichier)) || strlen(nomFichier) == 0) {
        strcpy(nomFichier, "schemes.txt");
    }
    
    int nb = chargerSchemes(ctx.schemes, nomFichier);
    if (nb > 0) {
        printf("\n");
    }
}

void menuAjouterScheme() {
    afficherTitre("➕ AJOUTER UN SCHÈME");
    
    printf("Nom du schème (ex: فاعل) : ");
    char nom[MAX_LEN];
    if (!lireLigne(nom, sizeof(nom))) {
        printf("❌ Erreur de lecture\n");
        return;
    }
    
    printf("Pattern (ex: 1ا23) : ");
    char pattern[MAX_LEN];
    if (!lireLigne(pattern, sizeof(pattern))) {
        printf("❌ Erreur de lecture\n");
        return;
    }
    
    Scheme s = creerScheme(nom, pattern);
    insererScheme(ctx.schemes, s);
    printf("✅ Schème '%s' ajouté avec succès\n\n", nom);
}

void menuModifierScheme() {
    afficherTitre("✏️  MODIFIER UN SCHÈME");
    
    const char* nom = selectionnerSchemeParNumero(ctx.schemes);
    if (!nom) {
        printf("❌ Aucun schème sélectionné\n\n");
        return;
    }
    
    printf("\nNouveau pattern : ");
    char pattern[MAX_LEN];
    if (!lireLigne(pattern, sizeof(pattern))) {
        printf("❌ Erreur de lecture\n");
        return;
    }
    
    Scheme s = creerScheme(nom, pattern);
    insererScheme(ctx.schemes, s);
    printf("✅ Schème '%s' modifié avec succès\n\n", nom);
}

void menuSupprimerScheme() {
    afficherTitre("🗑️  SUPPRIMER UN SCHÈME");
    
    const char* nom = selectionnerSchemeParNumero(ctx.schemes);
    if (!nom) {
        printf("❌ Aucun schème sélectionné\n\n");
        return;
    }
    
    printf("\n⚠️  Êtes-vous sûr de vouloir supprimer '%s' ? (o/n) : ", nom);
    char reponse[10];
    if (!lireLigne(reponse, sizeof(reponse))) {
        printf("❌ Annulation\n\n");
        return;
    }
    
    if (reponse[0] == 'o' || reponse[0] == 'O') {
        if (supprimerScheme(ctx.schemes, nom)) {
            printf("✅ Schème '%s' supprimé\n\n", nom);
        } else {
            printf("❌ Échec de la suppression\n\n");
        }
    } else {
        printf("❌ Suppression annulée\n\n");
    }
}

void menuStatistiquesArbre() {
    afficherTitre("📊 STATISTIQUES DE L'ARBRE AVL");
    displayBuffer[0] = '\0';
    afficherStatistiquesArbre(ctx.racines, appendToBuffer);
    flushBuffer();
}

void menuStatistiquesHash() {
    afficherTitre("📊 STATISTIQUES TABLE DE HACHAGE");
    displayBuffer[0] = '\0';
    afficherStatistiquesHash(ctx.schemes, appendToBuffer);
    flushBuffer();
}

void menuSauvegarderRacines() {
    afficherTitre("💾 SAUVEGARDE DES RACINES");
    printf("⚠️  Fonction non implémentée dans cette version\n");
    printf("   TODO : Parcours in-order de l'arbre + écriture fichier\n\n");
}

// -----------------------------------------------------------------------------
// Programme principal
// -----------------------------------------------------------------------------

int main(int argc, char *argv[]) {
    // Configuration UTF-8 pour l'arabe
    setlocale(LC_ALL, "");
    
    // Initialisation du contexte
    ctx.racines = NULL;
    ctx.schemes = creerTableHash();
    if (!ctx.schemes) {
        fprintf(stderr, "❌ Erreur fatale : impossible de créer la table de hachage\n");
        return 1;
    }

    // Message de bienvenue
    printf("\n");
    printf("╔═══════════════════════════════════════════════════════════╗\n");
    printf("║                                                           ║\n");
    printf("║         🌙 MOTEUR MORPHOLOGIQUE ARABE 🌙                 ║\n");
    printf("║              Mini-Projet Algorithmique                    ║\n");
    printf("║                                                           ║\n");
    printf("╚═══════════════════════════════════════════════════════════╝\n");
    printf("\n");

    // Initialisation des schèmes de base
    initialiserSchemesDeBase(ctx.schemes);

    // Choix du mode (console ou GUI)
    printf("Choisissez le mode d'utilisation :\n");
    printf("  1) Mode console (interface texte)\n");
    printf("  2) Mode graphique (GUI - si disponible)\n");
    printf("\nVotre choix : ");

    int mode;
    if (scanf("%d", &mode) != 1) {
        mode = 1;
    }
    viderBuffer();

    // Lancement du mode graphique si disponible
    if (mode == 2) {
        if (lancer_gui != NULL) {
            return lancer_gui(argc, argv);
        } else {
            printf("\n❌ Mode graphique non disponible (gui.c non compilé)\n");
            printf("   Passage en mode console...\n\n");
        }
    }

    // Boucle principale du menu console
    int choix;
    do {
        afficherMenu();
        
        if (scanf("%d", &choix) != 1) {
            viderBuffer();
            choix = -1;
        }
        viderBuffer();

        switch (choix) {
            case 1:  menuChargerRacines();        break;
            case 2:  menuAfficherRacines();       break;
            case 3:  menuAjouterRacine();         break;
            case 4:  menuRechercherRacine();      break;
            case 5:  menuGenererDerives();        break;
            case 6:  menuAfficherDerives();       break;
            case 7:  menuValiderMot();            break;
            case 8:  menuAfficherSchemes();       break;
            case 9:  menuChargerSchemes();        break;
            case 10: menuAjouterScheme();         break;
            case 11: menuModifierScheme();        break;
            case 12: menuSupprimerScheme();       break;
            case 13: menuStatistiquesArbre();     break;
            case 14: menuStatistiquesHash();      break;
            case 15: menuSauvegarderRacines();    break;
            
            case 0:
                printf("\n");
                afficherTitre("👋 AU REVOIR");
                printf("Merci d'avoir utilisé le moteur morphologique arabe !\n");
                printf("مع السلامة\n\n");
                break;
                
            default:
                printf("\n❌ Choix invalide. Veuillez entrer un nombre entre 0 et 15.\n\n");
        }

    } while (choix != 0);

    // Libération de la mémoire
    libererArbre(ctx.racines);
    libererTableHash(ctx.schemes);

    return 0;
}