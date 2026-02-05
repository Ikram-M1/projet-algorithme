#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#include "types.h"
#include "arbre.h"
#include "hash.h"
#include "morphologie.h"

// Charge les racines (en arabe ou translittération) depuis racines.txt
void chargerRacinesDepuisFichier(NoeudArbre** arbre, const char* nomFichier) {
    FILE* f = fopen(nomFichier, "r");
    if (!f) {
        printf("Impossible d'ouvrir %s\n", nomFichier);
        return;
    }

    char ligne[MAX_LEN];
    while (fgets(ligne, sizeof(ligne), f)) {
        // enlever \n
        ligne[strcspn(ligne, "\r\n")] = '\0';
        if (strlen(ligne) > 0) {
            *arbre = insererRacine(*arbre, ligne);
        }
    }

    fclose(f);
}

// Initialisation de quelques schèmes de base (noms + patterns)
void initialiserSchemesDeBase(TableHash* t) {
    // Ici les noms peuvent être en arabe si ton terminal les affiche bien,
    // les patterns restent en ASCII (1,2,3 pour les consonnes de la racine).

    // Exemple en translittération (pédagogique)
    Scheme s1 = creerScheme("fa3il", "1a2i3");      // schème type فاعل
    insererScheme(t, s1);

    Scheme s2 = creerScheme("maf3oul", "ma1u2u3");  // schème type مفعول
    insererScheme(t, s2);

    Scheme s3 = creerScheme("ifta3al", "i1ta2a3");  // schème type افتعل
    insererScheme(t, s3);

    Scheme s4 = creerScheme("taf3il", "ta1i2i3");   // schème type تفعيل
    insererScheme(t, s4);

    // Tu pourras plus tard remplacer les noms par "فاعل", "مفعول", etc.
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
    printf("0) Quitter\n");
    printf("Votre choix: ");
}

int main() {
    setlocale(LC_ALL, "");  // pour aider l'affichage UTF-8

    Contexte ctx;
    ctx.racines = NULL;
    ctx.schemes = creerTableHash();
    initialiserSchemesDeBase(ctx.schemes);

    int choix;
    char racine[MAX_LEN];
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

            case 3:
                printf("Entrer la racine: ");
                scanf("%s", racine);
                genererFamilleMorphologique(ctx.racines, ctx.schemes, racine);
                break;

            case 4: {
                printf("Entrer la racine: ");
                scanf("%s", racine);
                printf("Entrer le mot à vérifier: ");
                scanf("%s", mot);
                Scheme* s = NULL;
                if (validerMotPourRacine(ctx.schemes, mot, racine, &s)) {
                    printf("OUI, le mot appartient à la racine %s (schème: %s)\n",
                           racine, s ? s->nom : "inconnu");
                } else {
                    printf("NON, le mot ne correspond pas à la racine %s.\n", racine);
                }
                break;
            }

            case 5:
                afficherSchemes(ctx.schemes);
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
