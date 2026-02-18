#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#ifdef _WIN32
    #include <windows.h>
#endif

#include "types.h"
#include "arbre.h"
#include "hash.h"
#include "morphologie.h"

// Déclaration de la fonction GUI (à implémenter dans gui.c)
// extern void launch_gui(int argc, char *argv[], Contexte *ctx);   // déclaration

// // Dans le main, quand l'utilisateur choisit le mode GUI :
// if (mode == 2) {
//     launch_gui(argc, argv, &ctx);
//     return 0;
// }
// Charge les racines (en arabe ou translittération) depuis racines.txt
void chargerRacinesDepuisFichier(NoeudArbre** arbre, const char* nomFichier) {
    FILE* f = fopen(nomFichier, "r");
    if (!f) {
        printf("Impossible d'ouvrir %s\n", nomFichier);
        return;
    }

    char ligne[MAX_LEN];
    while (fgets(ligne, sizeof(ligne), f)) {
        ligne[strcspn(ligne, "\r\n")] = '\0';
        if (strlen(ligne) > 0) {
            *arbre = insererRacine(*arbre, ligne);
        }
    }

    fclose(f);
}

// Initialisation de quelques schèmes de base (noms + patterns en arabe)
void initialiserSchemesDeBase(TableHash* t) {
    Scheme s1 = creerScheme("فاعل", "1ا23");
    insererScheme(t, s1);

    Scheme s2 = creerScheme("مفعول", "م12و3");
    insererScheme(t, s2);

    Scheme s3 = creerScheme("افتعل", "ا1ت23");
    insererScheme(t, s3);

    Scheme s4 = creerScheme("تفعيل", "ت12ي3");
    insererScheme(t, s4);
}

void afficherMenu() {
    printf("====================================\n");
    printf("  MOTEUR MORPHOLOGIQUE ARABE (C) - VERSION 2.0\n");
    printf("====================================\n");
    printf("  GESTION DES RACINES\n");
    printf("  -----------------------------------------------\n");
    printf("  1) Charger les racines depuis fichier\n");
    printf("  2) Afficher toutes les racines\n");
    printf("  3) Ajouter une nouvelle racine\n");
    printf("\n  GÉNÉRATION ET VÉRIFICATION\n");
    printf("  -----------------------------------------------\n");
    printf("  4) Générer les dérivés d'une racine\n");
    printf("  5) Vérifier si un mot appartient à une racine (+ schème)\n");
    printf("  6) Décomposer un mot (identifier racine + schème)\n");
    printf("  7) Afficher les dérivés d'une racine\n");
    printf("\n  GESTION DES SCHÈMES\n");
    printf("  -----------------------------------------------\n");
    printf("  8) Afficher les schèmes disponibles\n");
    printf("  9) Ajouter un schème\n");
    printf("  10) Modifier un schème\n");
    printf("  11) Supprimer un schème\n");
    printf("\n  VERBES IRRÉGULIERS (أفعال غير منتظمة)\n");
    printf("  -----------------------------------------------\n");
    printf("  12 - Verbes Creux (أجوف)\n");
    printf("       a) Générer les dérivés d'une racine Ajwaf\n");
    printf("       b) Vérifier si un mot appartient à une racine Ajwaf\n");
    printf("       c) Décomposer un mot Ajwaf\n");
    printf("       d) Afficher les dérivés d'une racine Ajwaf\n");
    printf("\n  -----------------------------------------------\n");
    printf("  0) Quitter\n");
    printf("Votre choix: ");
}

int main(void) {
    // Configuration UTF-8 sur Windows
    #ifdef _WIN32
        SetConsoleCP(CP_UTF8);
        SetConsoleOutputCP(CP_UTF8);
    #endif
    
    setlocale(LC_ALL, ".UTF-8");  // forcer UTF-8

    printf("Choisissez le mode :\n");
    printf("1) Mode console\n");
    printf("2) Mode graphique (interface GTK)\n");
    printf("Votre choix : ");

    int mode;
    if (scanf("%d", &mode) != 1) {
        mode = 1;  // par défaut console
    }

    // Si mode graphique : lancer l'interface GTK
    // if (mode == 2) {
    //     return lancer_gui(argc, argv);
    // }

    // Sinon : mode console (ton code original)
    Contexte ctx;
    ctx.racines = NULL;
    ctx.schemes = creerTableHash();
    if (!ctx.schemes) return 1;
    initialiserSchemesDeBase(ctx.schemes);

    int choix;
    char buffer[MAX_LEN];
    char mot[MAX_LEN];

    do {
        afficherMenu();
        if (scanf("%d", &choix) != 1) {
            // vider l'entrée si saisie invalide
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            choix = -1;
        }

        switch (choix) {
            case 1:
                chargerRacinesDepuisFichier(&ctx.racines, "racines.txt");
                printf("Racines chargées.\n");
                break;

            case 2:
                afficherRacines(ctx.racines);
                break;

            case 4: {
                const char* racineChoisie = selectionnerRacineParNumero(ctx.racines);
                if (racineChoisie) {
                    printf("\nGénération des dérivés pour la racine sélectionnée : %s\n", racineChoisie);
                    genererFamilleMorphologique(ctx.racines, ctx.schemes, racineChoisie);
                } else {
                    printf("Aucune racine sélectionnée.\n");
                }
                break;
            }

            case 5: {
                printf("========================================\n");
                printf("Vérifier si un mot appartient à une racine\n");
                printf("========================================\n");
                printf("Entrer la racine: ");
                scanf("%s", buffer);
                printf("Entrer le mot à vérifier: ");
                scanf("%s", mot);
                Scheme* s = NULL;
                if (validerMotPourRacine(ctx.racines, ctx.schemes, mot, buffer, &s)) {
                    printf("\n✓ OUI, le mot appartient à la racine.\n");
                    printf("  Racine: %s\n", buffer);
                    printf("  Mot: %s\n", mot);
                    if (s) printf("  Schème utilisé: %s\n", s->nom);
                } else {
                    printf("\n✗ NON, le mot ne correspond pas à la racine.\n");
                    printf("  Racine testée: %s\n", buffer);
                    printf("  Mot testé: %s\n", mot);
                }
                break;
            }

            case 8:
                afficherSchemes(ctx.schemes);
                break;

            case 3:
                printf("Entrer la nouvelle racine: ");
                scanf("%s", buffer);
                ctx.racines = insererRacine(ctx.racines, buffer);
                printf("Racine ajoutée.\n");
                break;

            case 9: {
                char nom[MAX_LEN], pattern[MAX_LEN];
                printf("Entrer le nom du schème: ");
                scanf("%s", nom);
                printf("Entrer le pattern (ex. م12و3): ");
                scanf("%s", pattern);
                Scheme s = creerScheme(nom, pattern);
                insererScheme(ctx.schemes, s);
                printf("Schème ajouté.\n");
                break;
            }

            case 10: {
                char nom[MAX_LEN], new_pattern[MAX_LEN];
                printf("Entrer le nom du schème à modifier: ");
                scanf("%s", nom);
                if (chercherScheme(ctx.schemes, nom)) {
                    printf("Entrer le nouveau pattern: ");
                    scanf("%s", new_pattern);
                    Scheme s = creerScheme(nom, new_pattern);
                    insererScheme(ctx.schemes, s);  // Remplace
                    printf("Schème modifié.\n");
                } else {
                    printf("Schème introuvable.\n");
                }
                break;
            }

            case 11:
                printf("Entrer le nom du schème à supprimer: ");
                scanf("%s", buffer);
                supprimerScheme(ctx.schemes, buffer);
                printf("Schème supprimé si existant.\n");
                break;

            case 7:
                printf("========================================\n");
                printf("Afficher les dérivés d'une racine\n");
                printf("========================================\n");
                printf("Entrer la racine: ");
                scanf("%s", buffer);
                NoeudArbre* n = rechercherRacine(ctx.racines, buffer);
                if (n) {
                    printf("\n");
                    afficherDerivesRacine(&n->data);
                    printf("\n");
                } else {
                    printf("✗ Racine introuvable.\n");
                }
                break;

            case 6: {
                printf("========================================\n");
                printf("Décomposition du mot\n");
                printf("========================================\n");
                printf("Entrer le mot à décomposer: ");
                scanf("%s", mot);
                
                char racineTrouvee[MAX_LEN] = {0};
                char schemeTrouve[MAX_LEN] = {0};
                
                if (decomposerMot(ctx.racines, ctx.schemes, mot, racineTrouvee, schemeTrouve)) {
                    printf("\n✓ Décomposition réussie!\n");
                    printf("====================================\n");
                    printf("Mot: %s\n", mot);
                    printf("Racine identifiée: %s\n", racineTrouvee);
                    printf("Schème utilisé: %s\n", schemeTrouve);
                    printf("====================================\n");
                } else {
                    printf("\n✗ Décomposition impossible.\n");
                    printf("====================================\n");
                    printf("Le mot '%s' n'a pas pu être décomposé.\n", mot);
                    printf("Vérifiez que:\n");
                    printf("  • Les racines ont été chargées (option 1)\n");
                    printf("  • Le mot est un dérivé valide\n");
                    printf("====================================\n");
                }
                break;
            }

            case 0:
                printf("Au revoir.\n");
                break;

            case 12: {
                printf("\n====================================\n");
                printf("  VERBES CREUX (أجوف - AJWAF)\n");
                printf("====================================\n");
                printf("  a) Générer les dérivés d'une racine Ajwaf\n");
                printf("  b) Vérifier si un mot appartient à une racine Ajwaf\n");
                printf("  c) Décomposer un mot Ajwaf\n");
                printf("  d) Afficher les dérivés d'une racine Ajwaf\n");
                printf("  0) Retour au menu principal\n");
                printf("Votre choix (a/b/c/d/0): ");

                char saisie[10];
                scanf("%s", saisie);

                if (strcmp(saisie, "a") == 0) {
                    // Générer les dérivés d'une racine Ajwaf
                    const char* racineChoisie = selectionnerRacineParNumero(ctx.racines);
                    if (racineChoisie) {
                        if (estAjwaf(racineChoisie)) {
                            printf("\n✓ La racine %s est de type Ajwaf\n", racineChoisie);
                            printf("Génération des dérivés pour cette racine Ajwaf:\n");
                            genererFamilleAjwaf(ctx.racines, ctx.schemes, racineChoisie);
                        } else {
                            printf("\n✗ La racine %s n'est pas de type Creux (Ajwaf).\n", racineChoisie);
                            printf("Pour être Ajwaf, la deuxième lettre (R2) doit être و (waw) ou ي (ya).\n");
                        }
                    } else {
                        printf("Aucune racine sélectionnée.\n");
                    }
                }
                else if (strcmp(saisie, "b") == 0) {
                    // Vérifier si un mot appartient à une racine Ajwaf
                    printf("========================================\n");
                    printf("Vérifier un mot avec racine Ajwaf\n");
                    printf("========================================\n");
                    printf("Entrer la racine: ");
                    scanf("%s", buffer);
                    
                    if (!estAjwaf(buffer)) {
                        printf("✗ La racine %s n'est pas de type Creux (Ajwaf).\n", buffer);
                    } else {
                        printf("Entrer le mot à vérifier: ");
                        scanf("%s", mot);
                        
                        Scheme* s = NULL;
                        if (validerMotPourRacineAjwaf(ctx.racines, ctx.schemes, mot, buffer, &s)) {
                            printf("\n✓ OUI, le mot appartient à la racine Ajwaf.\n");
                            printf("  Racine: %s\n", buffer);
                            printf("  Mot: %s\n", mot);
                            if (s) printf("  Schème utilisé: %s\n", s->nom);
                        } else {
                            printf("\n✗ NON, le mot ne correspond pas à la racine.\n");
                            printf("  Racine testée: %s\n", buffer);
                            printf("  Mot testé: %s\n", mot);
                        }
                    }
                }
                else if (strcmp(saisie, "c") == 0) {
                    // Décomposer un mot Ajwaf
                    printf("========================================\n");
                    printf("Décomposition d'un mot Ajwaf\n");
                    printf("========================================\n");
                    printf("Entrer le mot à décomposer: ");
                    scanf("%s", mot);
                    
                    char racineTrouvee[MAX_LEN] = {0};
                    char schemeTrouve[MAX_LEN] = {0};
                    
                    if (decomposerMotAjwaf(ctx.racines, ctx.schemes, mot, racineTrouvee, schemeTrouve)) {
                        printf("\n✓ Décomposition réussie (verbe Ajwaf)!\n");
                        printf("====================================\n");
                        printf("Mot: %s\n", mot);
                        printf("Racine identifiée: %s (Ajwaf)\n", racineTrouvee);
                        printf("Schème utilisé: %s\n", schemeTrouve);
                        printf("====================================\n");
                    } else {
                        printf("\n✗ Décomposition impossible.\n");
                        printf("====================================\n");
                        printf("Le mot '%s' n'a pas pu être décomposé.\n", mot);
                        printf("Vérifiez que:\n");
                        printf("  • Les racines ont été chargées (option 1)\n");
                        printf("  • Le mot est un dérivé valide d'une racine Ajwaf\n");
                        printf("  • La deuxième lettre de la racine est و ou ي\n");
                        printf("====================================\n");
                    }
                }
                else if (strcmp(saisie, "d") == 0) {
                    // Afficher les dérivés d'une racine Ajwaf
                    printf("========================================\n");
                    printf("Afficher les dérivés d'une racine Ajwaf\n");
                    printf("========================================\n");
                    printf("Entrer la racine: ");
                    scanf("%s", buffer);
                    
                    if (!estAjwaf(buffer)) {
                        printf("✗ La racine %s n'est pas de type Creux (Ajwaf).\n", buffer);
                    } else {
                        NoeudArbre* n = rechercherRacine(ctx.racines, buffer);
                        if (n) {
                            printf("\n");
                            afficherDerivesRacine(&n->data);
                            printf("\n");
                        } else {
                            printf("✗ Racine introuvable.\n");
                        }
                    }
                }
                else if (strcmp(saisie, "0") == 0) {
                    // Retour au menu principal - rien à faire
                }
                else {
                    printf("Choix inexistant.\n");
                }
                break;
            }

            default:
                printf("Choix invalide.\n");
        }

    } while (choix != 0);

    libererArbre(ctx.racines);
    libererTableHash(ctx.schemes);

    return 0;
}