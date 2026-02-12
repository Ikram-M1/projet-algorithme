#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

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
    printf("  MOTEUR MORPHOLOGIQUE ARABE (C)\n");
    printf("====================================\n");
    printf("1) Charger les racines depuis fichier\n");
    printf("2) Afficher toutes les racines\n");
    printf("3) Générer les dérivés d'une racine\n");
    printf("4) Vérifier si un mot appartient à une racine\n");
    printf("5) Afficher les schèmes disponibles\n");
    printf("6) Ajouter une nouvelle racine\n");
    printf("7) Ajouter un schème\n");
    printf("8) Modifier un schème\n");
    printf("9) Supprimer un schème\n");
    printf("10) Afficher les dérivés d'une racine\n");
    printf("0) Quitter\n");
    printf("Votre choix: ");
}

int main(int argc, char *argv[]) {
    setlocale(LC_ALL, "");  // pour aider l'affichage UTF-8

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

            case 3: {
                const char* racineChoisie = selectionnerRacineParNumero(ctx.racines);
                if (racineChoisie) {
                    printf("\nGénération des dérivés pour la racine sélectionnée : %s\n", racineChoisie);
                    genererFamilleMorphologique(ctx.racines, ctx.schemes, racineChoisie);
                } else {
                    printf("Aucune racine sélectionnée.\n");
                }
                break;
            }

            case 4: {
                printf("Entrer la racine: ");
                scanf("%s", buffer);
                printf("Entrer le mot à vérifier: ");
                scanf("%s", mot);
                Scheme* s = NULL;
                if (validerMotPourRacine(ctx.racines, ctx.schemes, mot, buffer, &s)) {
                    printf("OUI, le mot appartient à la racine %s (schème: %s)\n",
                           buffer, s ? s->nom : "inconnu");
                } else {
                    printf("NON, le mot ne correspond pas à la racine %s.\n", buffer);
                }
                break;
            }

            case 5:
                afficherSchemes(ctx.schemes);
                break;

            case 6:
                printf("Entrer la nouvelle racine: ");
                scanf("%s", buffer);
                ctx.racines = insererRacine(ctx.racines, buffer);
                printf("Racine ajoutée.\n");
                break;

            case 7: {
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

            case 8: {
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

            case 9:
                printf("Entrer le nom du schème à supprimer: ");
                scanf("%s", buffer);
                supprimerScheme(ctx.schemes, buffer);
                printf("Schème supprimé si existant.\n");
                break;

            case 10:
                printf("Entrer la racine: ");
                scanf("%s", buffer);
                NoeudArbre* n = rechercherRacine(ctx.racines, buffer);
                if (n) afficherDerivesRacine(&n->data);
                else printf("Racine introuvable.\n");
                break;

            case 0:
                printf("Au revoir.\n");
                break;

            default:
                printf("Choix invalide.\n");
        }

    } while (choix != 0);

    libererArbre(ctx.racines);
    libererTableHash(ctx.schemes);

    return 0;
}