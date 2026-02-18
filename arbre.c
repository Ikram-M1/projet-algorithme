    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <locale.h>  // Pour le support UTF-8
    #include "arbre.h"
    #include "types.h"

    // -----------------------------------------------------------------------------
    // Fonctions utilitaires
    // -----------------------------------------------------------------------------

    /**
    * Vérifie si une chaîne contient des caractères arabes UTF-8
    * Retourne 1 si arabe, 0 sinon
    */
    static int estChaineArabe(const char* str) {
        if (!str || strlen(str) == 0) return 0;
        
        // Vérifie la présence de caractères UTF-8 arabes (U+0600 à U+06FF)
        const unsigned char* p = (const unsigned char*)str;
        while (*p) {
            // Détection basique UTF-8 arabe (commence par 0xD8 ou 0xD9)
            if ((*p == 0xD8 || *p == 0xD9) && *(p+1) >= 0x80) {
                return 1;
            }
            p++;
        }
        return 1;  // On accepte même si détection échoue (pour compatibilité)
    }

    /**
    * Nettoie les espaces en début/fin de chaîne
    */
    static void trimString(char* str) {
        if (!str) return;
        
        // Supprimer espaces à gauche
        char* start = str;
        while (*start && (*start == ' ' || *start == '\t' || *start == '\n' || *start == '\r')) {
            start++;
        }
        
        // Supprimer espaces à droite
        char* end = start + strlen(start) - 1;
        while (end > start && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) {
            end--;
        }
        *(end + 1) = '\0';
        
        // Déplacer si nécessaire
        if (start != str) {
            memmove(str, start, strlen(start) + 1);
        }
    }

    // -----------------------------------------------------------------------------
    // Fonctions de base AVL
    // -----------------------------------------------------------------------------

    NoeudArbre* creerNoeud(const char* racineStr) {
        if (!racineStr || strlen(racineStr) == 0) {
            fprintf(stderr, "Erreur : racine vide lors de la création du noeud\n");
            return NULL;
        }

        NoeudArbre* n = (NoeudArbre*)malloc(sizeof(NoeudArbre));
        if (!n) {
            fprintf(stderr, "Erreur : allocation mémoire échouée pour le noeud\n");
            return NULL;
        }

        // Copie sécurisée avec nettoyage
        char temp[MAX_LEN];
        strncpy(temp, racineStr, MAX_LEN - 1);
        temp[MAX_LEN - 1] = '\0';
        trimString(temp);
        
        strcpy(n->data.racine, temp);
        n->data.derives = NULL;
        n->data.nbDerives = 0;
        n->gauche = n->droite = NULL;
        n->hauteur = 1;
        
        return n;
    }

    int max(int a, int b) {
        return a > b ? a : b;
    }

    int hauteur(NoeudArbre* n) {
        return n ? n->hauteur : 0;
    }

    int equilibre(NoeudArbre* n) {
        return n ? hauteur(n->gauche) - hauteur(n->droite) : 0;
    }

    NoeudArbre* rotationDroite(NoeudArbre* y) {
        if (!y || !y->gauche) return y;  // Sécurité
        
        NoeudArbre* x = y->gauche;
        NoeudArbre* T2 = x->droite;

        // Effectuer la rotation
        x->droite = y;
        y->gauche = T2;

        // Mettre à jour les hauteurs
        y->hauteur = max(hauteur(y->gauche), hauteur(y->droite)) + 1;
        x->hauteur = max(hauteur(x->gauche), hauteur(x->droite)) + 1;

        return x;
    }

    NoeudArbre* rotationGauche(NoeudArbre* x) {
        if (!x || !x->droite) return x;  // Sécurité
        
        NoeudArbre* y = x->droite;
        NoeudArbre* T2 = y->gauche;

        // Effectuer la rotation
        y->gauche = x;
        x->droite = T2;

        // Mettre à jour les hauteurs
        x->hauteur = max(hauteur(x->gauche), hauteur(x->droite)) + 1;
        y->hauteur = max(hauteur(y->gauche), hauteur(y->droite)) + 1;

        return y;
    }

    // -----------------------------------------------------------------------------
    // Insertion avec équilibrage AVL
    // -----------------------------------------------------------------------------

    NoeudArbre* insererRacine(NoeudArbre* racine, const char* nouvelleRacine) {
        // Validation de l'entrée
        if (!nouvelleRacine || strlen(nouvelleRacine) == 0) {
            fprintf(stderr, "Erreur : tentative d'insertion d'une racine vide\n");
            return racine;
        }

        // Nettoyage de la racine
        char racineClean[MAX_LEN];
        strncpy(racineClean, nouvelleRacine, MAX_LEN - 1);
        racineClean[MAX_LEN - 1] = '\0';
        trimString(racineClean);

        // Cas de base : arbre vide
        if (racine == NULL) {
            return creerNoeud(racineClean);
        }

        // Insertion récursive
        int cmp = strcmp(racineClean, racine->data.racine);
        if (cmp < 0) {
            racine->gauche = insererRacine(racine->gauche, racineClean);
        } else if (cmp > 0) {
            racine->droite = insererRacine(racine->droite, racineClean);
        } else {
            // Racine déjà présente
            return racine;
        }

        // Mise à jour de la hauteur du noeud ancêtre
        racine->hauteur = 1 + max(hauteur(racine->gauche), hauteur(racine->droite));

        // Calcul du facteur d'équilibre
        int eq = equilibre(racine);

        // Cas 1 : Left-Left (LL)
        if (eq > 1 && strcmp(racineClean, racine->gauche->data.racine) < 0) {
            return rotationDroite(racine);
        }

        // Cas 2 : Left-Right (LR)
        if (eq > 1 && strcmp(racineClean, racine->gauche->data.racine) > 0) {
            racine->gauche = rotationGauche(racine->gauche);
            return rotationDroite(racine);
        }

        // Cas 3 : Right-Right (RR)
        if (eq < -1 && strcmp(racineClean, racine->droite->data.racine) > 0) {
            return rotationGauche(racine);
        }

        // Cas 4 : Right-Left (RL)
        if (eq < -1 && strcmp(racineClean, racine->droite->data.racine) < 0) {
            racine->droite = rotationDroite(racine->droite);
            return rotationGauche(racine);
        }

        return racine;
    }

    // -----------------------------------------------------------------------------
    // Recherche
    // -----------------------------------------------------------------------------

    NoeudArbre* rechercherRacine(NoeudArbre* racine, const char* cle) {
        if (!cle) return NULL;
        if (racine == NULL) return NULL;

        // Nettoyage de la clé de recherche
        char cleClean[MAX_LEN];
        strncpy(cleClean, cle, MAX_LEN - 1);
        cleClean[MAX_LEN - 1] = '\0';
        trimString(cleClean);

        int cmp = strcmp(cleClean, racine->data.racine);
        if (cmp == 0) return racine;
        if (cmp < 0) return rechercherRacine(racine->gauche, cleClean);
        return rechercherRacine(racine->droite, cleClean);
    }

    // -----------------------------------------------------------------------------
    // Affichage trié alphabétiquement (collecte + qsort)
    // -----------------------------------------------------------------------------

    static int comparerRacines(const void *a, const void *b) {
        return strcmp(*(const char **)a, *(const char **)b);
    }

    static int compterNoeuds(NoeudArbre *n) {
        if (n == NULL) return 0;
        return 1 + compterNoeuds(n->gauche) + compterNoeuds(n->droite);
    }

    static void remplirTableau(NoeudArbre *n, const char **tab, int *idx) {
        if (n == NULL) return;
        remplirTableau(n->gauche, tab, idx);
        tab[*idx] = n->data.racine;
        (*idx)++;
        remplirTableau(n->droite, tab, idx);
    }

    // Version callback de afficherRacines (propre et fiable)
    void afficherRacinesCallback(NoeudArbre *racine, AppendFunc append) {
        if (!append) {
            fprintf(stderr, "Erreur : fonction callback NULL\n");
            return;
        }

        if (racine == NULL) {
            append("⚠️  Aucune racine stockée dans l'arbre.\n");
            return;
        }

        int count = compterNoeuds(racine);
        if (count == 0) {
            append("⚠️  Aucune racine stockée.\n");
            return;
        }

        const char **tab = malloc(count * sizeof(const char *));
        if (!tab) {
            append("❌ Erreur : allocation mémoire échouée.\n");
            return;
        }

        int idx = 0;
        remplirTableau(racine, tab, &idx);
        qsort(tab, count, sizeof(const char *), comparerRacines);

        append("\n╔═══════════════════════════════════════════════════╗\n");
        append("║   📚 RACINES STOCKÉES (ordre alphabétique)       ║\n");
        append("╚═══════════════════════════════════════════════════╝\n\n");

        for (int i = 0; i < count; i++) {
            char line[150];
            snprintf(line, sizeof(line), "  %3d. %s\n", i + 1, tab[i]);
            append(line);
        }

        char footer[100];
        snprintf(footer, sizeof(footer), "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n📊 Total : %d racine(s)\n\n", count);
        append(footer);

        free(tab);
    }

    // -----------------------------------------------------------------------------
    // Gestion des dérivés
    // -----------------------------------------------------------------------------

    /**
    * Ajoute un dérivé à la liste d'une racine (sans doublons)
    * Incrémente la fréquence si le mot existe déjà
    */
    void ajouterDerive(NoeudArbre* noeud, const char* motDerive) {
        if (!noeud) {
            fprintf(stderr, "Erreur : noeud NULL dans ajouterDerive\n");
            return;
        }
        
        if (!motDerive || strlen(motDerive) == 0) {
            fprintf(stderr, "Erreur : mot dérivé vide\n");
            return;
        }

        // Nettoyage du mot
        char motClean[MAX_LEN];
        strncpy(motClean, motDerive, MAX_LEN - 1);
        motClean[MAX_LEN - 1] = '\0';
        trimString(motClean);

        // Vérifier si déjà présent
        Derive* courant = noeud->data.derives;
        while (courant) {
            if (strcmp(courant->mot, motClean) == 0) {
                courant->frequence++;
                return;
            }
            courant = courant->suivant;
        }

        // Ajout d'un nouveau dérivé
        Derive* nouveau = (Derive*)malloc(sizeof(Derive));
        if (!nouveau) {
            fprintf(stderr, "Erreur : allocation mémoire échouée pour le dérivé\n");
            return;
        }

        strcpy(nouveau->mot, motClean);
        nouveau->frequence = 1;
        nouveau->suivant = noeud->data.derives;
        noeud->data.derives = nouveau;
        noeud->data.nbDerives++;
    }

    /**
    * Affiche les dérivés d'une racine spécifique avec formatage amélioré
    */
    void afficherDerivesRacine(NoeudArbre* noeud, AppendFunc append) {
        if (!append) {
            fprintf(stderr, "Erreur : fonction callback NULL\n");
            return;
        }

        if (!noeud || noeud->data.nbDerives == 0) {
            char msg[200];
            if (noeud) {
                snprintf(msg, sizeof(msg), "\n⚠️  Aucun dérivé pour la racine '%s'\n\n", noeud->data.racine);
            } else {
                snprintf(msg, sizeof(msg), "\n⚠️  Racine inexistante\n\n");
            }
            append(msg);
            return;
        }

        char line[250];
        snprintf(line, sizeof(line), 
                "\n╔═══════════════════════════════════════════════════╗\n"
                "║   📖 Dérivés de : %s\n"
                "╚═══════════════════════════════════════════════════╝\n\n", 
                noeud->data.racine);
        append(line);

        Derive* courant = noeud->data.derives;
        int i = 1;
        while (courant) {
            snprintf(line, sizeof(line), "  %3d. %-30s (fréquence: %d)\n", 
                    i, courant->mot, courant->frequence);
            append(line);
            courant = courant->suivant;
            i++;
        }
        
        snprintf(line, sizeof(line), 
                "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n"
                "📊 Total : %d dérivé(s)\n\n", 
                noeud->data.nbDerives);
        append(line);
    }

    // -----------------------------------------------------------------------------
    // Statistiques de l'arbre
    // -----------------------------------------------------------------------------

    /**
    * Compte le nombre total de dérivés dans tout l'arbre
    */
    int compterDerivesTotaux(NoeudArbre* racine) {
        if (!racine) return 0;
        
        return racine->data.nbDerives + 
            compterDerivesTotaux(racine->gauche) + 
            compterDerivesTotaux(racine->droite);
    }

    /**
    * Affiche les statistiques globales de l'arbre
    */
    void afficherStatistiquesArbre(NoeudArbre* racine, AppendFunc append) {
        if (!append) return;
        
        int nbRacines = compterNoeuds(racine);
        int nbDerives = compterDerivesTotaux(racine);
        int hauteurMax = hauteur(racine);
        
        char stats[400];
        snprintf(stats, sizeof(stats),
                "\n╔═══════════════════════════════════════════════════╗\n"
                "║          📊 STATISTIQUES DE L'ARBRE              ║\n"
                "╚═══════════════════════════════════════════════════╝\n\n"
                "  🌳 Nombre de racines     : %d\n"
                "  📖 Nombre de dérivés     : %d\n"
                "  📏 Hauteur de l'arbre    : %d\n"
                "  📈 Moyenne dérivés/racine: %.2f\n"
                "  ⚖️  Facteur d'équilibre   : %d\n\n",
                nbRacines, nbDerives, hauteurMax,
                nbRacines > 0 ? (float)nbDerives / nbRacines : 0.0,
                equilibre(racine));
        append(stats);
    }

    // -----------------------------------------------------------------------------
    // Libération mémoire
    // -----------------------------------------------------------------------------

    void libererDerives(Derive* d) {
        while (d) {
            Derive* tmp = d;
            d = d->suivant;
            free(tmp);
        }
    }

    void libererArbre(NoeudArbre* racine) {
        if (!racine) return;
        libererArbre(racine->gauche);
        libererArbre(racine->droite);
        libererDerives(racine->data.derives);
        free(racine);
    }

    // -----------------------------------------------------------------------------
    // Sélection interactive
    // -----------------------------------------------------------------------------

    /**
    * Retourne la racine sélectionnée par numéro (1-based), ou NULL si choix invalide
    */
    const char* selectionnerRacineParNumero(NoeudArbre* racine) {
        if (!racine) {
            printf("⚠️  Aucune racine disponible.\n");
            return NULL;
        }

        int count = compterNoeuds(racine);
        if (count == 0) {
            printf("⚠️  L'arbre est vide.\n");
            return NULL;
        }

        const char** tab = malloc(count * sizeof(const char*));
        if (!tab) {
            printf("❌ Erreur : allocation mémoire échouée.\n");
            return NULL;
        }

        int idx = 0;
        remplirTableau(racine, tab, &idx);
        qsort(tab, count, sizeof(const char*), comparerRacines);

        printf("\n╔═══════════════════════════════════════════════════╗\n");
        printf("║        🎯 SÉLECTION D'UNE RACINE                 ║\n");
        printf("╚═══════════════════════════════════════════════════╝\n\n");

        for (int i = 0; i < count; i++) {
            printf("  %3d. %s\n", i + 1, tab[i]);
        }
        
        printf("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
        printf("📝 Entrez le numéro (1 à %d) : ", count);

        int choix;
        if (scanf("%d", &choix) != 1) {
            while (getchar() != '\n');  // Vider le buffer
            printf("❌ Entrée invalide.\n");
            free(tab);
            return NULL;
        }
        
        if (choix < 1 || choix > count) {
            printf("❌ Numéro hors limites.\n");
            free(tab);
            return NULL;
        }

        const char* selectionnee = tab[choix - 1];
        free(tab);

        return selectionnee;
    }