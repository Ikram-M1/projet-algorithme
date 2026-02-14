// gui.c — INTERFACE GTK3 COMPLÈTE AVEC 2 NOUVEAUX BOUTONS
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <locale.h>

#include "types.h"
#include "arbre.h"
#include "hash.h"
#include "morphologie.h"

// Contexte global (passé depuis main)
extern Contexte ctx;

// Widgets globaux
static GtkWidget *result_text = NULL;
static GtkWidget *combo_racines = NULL;

// -----------------------------------------------------------------------------
// Fonctions utilitaires d'affichage
// -----------------------------------------------------------------------------

static void clear_text(void) {
    if (!result_text) return;
    
    GtkTextBuffer *buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(result_text));
    gtk_text_buffer_set_text(buf, "", -1);
}

static void append_text(const char *text) {
    if (!result_text || !text) return;
    
    GtkTextBuffer *buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(result_text));
    GtkTextIter end;
    gtk_text_buffer_get_end_iter(buf, &end);
    gtk_text_buffer_insert(buf, &end, text, -1);
    
    GtkTextMark *mark = gtk_text_buffer_get_insert(buf);
    gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(result_text),
                                 mark, 0.0, TRUE, 0.5, 1.0);
}

static void append_result(const char *text) {
    if (!text) return;
    
    char *formatted = g_strdup_printf("\u200F%s\u200E\n", text);
    append_text(formatted);
    g_free(formatted);
}

// -----------------------------------------------------------------------------
// Fonction pour remplir la liste déroulante des racines
// -----------------------------------------------------------------------------

static int compterNoeudsLocal(NoeudArbre *n) {
    if (n == NULL) return 0;
    return 1 + compterNoeudsLocal(n->gauche) + compterNoeudsLocal(n->droite);
}

static void remplirTableauLocal(NoeudArbre *n, const char **tab, int *idx) {
    if (n == NULL) return;
    remplirTableauLocal(n->gauche, tab, idx);
    tab[*idx] = n->data.racine;
    (*idx)++;
    remplirTableauLocal(n->droite, tab, idx);
}

static int comparerRacinesLocal(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

static void remplir_combo_racines(void) {
    if (!combo_racines) return;
    
    gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(combo_racines));
    
    if (!ctx.racines) {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_racines), 
                                       "(Aucune racine - Chargez d'abord)");
        gtk_combo_box_set_active(GTK_COMBO_BOX(combo_racines), 0);
        return;
    }
    
    int count = compterNoeudsLocal(ctx.racines);
    if (count == 0) {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_racines), 
                                       "(Aucune racine)");
        gtk_combo_box_set_active(GTK_COMBO_BOX(combo_racines), 0);
        return;
    }
    
    const char **tab = malloc(count * sizeof(const char *));
    if (!tab) return;
    
    int idx = 0;
    remplirTableauLocal(ctx.racines, tab, &idx);
    qsort(tab, count, sizeof(const char *), comparerRacinesLocal);
    
    for (int i = 0; i < count; i++) {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_racines), tab[i]);
    }
    
    free(tab);
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo_racines), 0);
}

// -----------------------------------------------------------------------------
// Callbacks des boutons EXISTANTS
// -----------------------------------------------------------------------------

static void on_load_clicked(GtkButton *btn, gpointer data) {
    (void)btn;
    (void)data;
    
    clear_text();
    append_result("📂 Chargement des racines...");
    
    chargerRacinesDepuisFichier(&ctx.racines, "racines.txt");
    append_result("✅ Racines chargées depuis racines.txt");
    
    remplir_combo_racines();
}

static void on_save_root_clicked(GtkButton *btn, gpointer entry) {
    (void)btn;
    
    const char *racine = gtk_entry_get_text(GTK_ENTRY(entry));
    
    if (strlen(racine) == 0) {
        clear_text();
        append_result("⚠️  Veuillez entrer une racine à enregistrer !");
        return;
    }
    
    size_t len = strlen(racine);
    
    if (len != 6 && len != 8 && len != 10) {
        clear_text();
        append_result("⚠️  Erreur : la racine doit être trilitère (3 lettres) ou quadrilitère (4 lettres)");
        char buf[256];
        snprintf(buf, sizeof(buf), "   '%s' a %zu octets", racine, len);
        append_result(buf);
        return;
    }
    
    FILE *f = fopen("racines.txt", "a");
    if (!f) {
        clear_text();
        append_result("❌ Erreur : impossible d'ouvrir racines.txt");
        return;
    }
    
    fprintf(f, "%s\n", racine);
    fclose(f);
    
    ctx.racines = insererRacine(ctx.racines, racine);
    
    clear_text();
    char buf[256];
    snprintf(buf, sizeof(buf), "✅ Racine '%s' enregistrée dans racines.txt", racine);
    append_result(buf);
    append_result("✅ Racine ajoutée à l'arbre en mémoire");
    
    gtk_entry_set_text(GTK_ENTRY(entry), "");
    remplir_combo_racines();
}

static void on_show_roots_clicked(GtkButton *btn, gpointer data) {
    (void)btn;
    (void)data;
    
    clear_text();
    append_text("╔═══════════════════════════════════════════════════════════╗\n");
    append_text("║            📚 LISTE DES RACINES STOCKÉES                 ║\n");
    append_text("╚═══════════════════════════════════════════════════════════╝\n\n");
    
    if (ctx.racines == NULL) {
        append_result("⚠️  Aucune racine chargée.");
        return;
    }
    
    afficherRacinesCallback(ctx.racines, append_text);
}

static void on_generate_clicked(GtkButton *btn, gpointer entry) {
    (void)btn;
    
    const char *racine = gtk_entry_get_text(GTK_ENTRY(entry));
    if (strlen(racine) == 0) {
        clear_text();
        append_result("⚠️  Veuillez entrer une racine !");
        return;
    }
    
    clear_text();
    
    char buf[256];
    snprintf(buf, sizeof(buf), "🔄 Génération des dérivés pour : %s\n", racine);
    append_result(buf);
    
    genererFamilleMorphologiqueCallback(ctx.racines, ctx.schemes, racine, append_text);
}

static void on_validate_clicked(GtkButton *btn, gpointer entry) {
    (void)btn;
    
    const char *mot = gtk_entry_get_text(GTK_ENTRY(entry));
    
    if (strlen(mot) == 0) {
        clear_text();
        append_result("⚠️  Veuillez entrer un mot à valider !");
        append_result("");
        append_result("💡 Instructions :");
        append_result("   1. Entrez un mot arabe dans le champ");
        append_result("   2. Sélectionnez une racine dans la liste déroulante");
        append_result("   3. Cliquez sur 'Valider mot'");
        return;
    }
    
    gchar *racine_selectionnee = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo_racines));
    
    if (!racine_selectionnee || 
        strcmp(racine_selectionnee, "(Aucune racine - Chargez d'abord)") == 0 ||
        strcmp(racine_selectionnee, "(Aucune racine)") == 0) {
        clear_text();
        append_result("⚠️  Veuillez d'abord charger les racines !");
        append_result("   Cliquez sur '📂 Charger racines'");
        g_free(racine_selectionnee);
        return;
    }
    
    clear_text();
    append_text("╔═══════════════════════════════════════════════════════════╗\n");
    append_text("║              ✅ VALIDATION MORPHOLOGIQUE                 ║\n");
    append_text("╚═══════════════════════════════════════════════════════════╝\n\n");
    
    char info[512];
    snprintf(info, sizeof(info), "🔍 Vérification : '%s' appartient-il à '%s' ?\n", 
             mot, racine_selectionnee);
    append_result(info);
    
    Scheme *sch = NULL;
    int ok = validerMotPourRacine(ctx.racines, ctx.schemes, mot, racine_selectionnee, &sch);
    
    char buf[512];
    if (ok) {
        append_text("\n");
        snprintf(buf, sizeof(buf), "✅ RÉSULTAT : OUI\n");
        append_text(buf);
        append_text("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");
        
        snprintf(buf, sizeof(buf), "   ✓ Le mot '%s' appartient bien à la racine '%s'\n", 
                 mot, racine_selectionnee);
        append_result(buf);
        
        if (sch) {
            snprintf(buf, sizeof(buf), "   📐 Schème identifié : %s", sch->nom);
            append_result(buf);
            snprintf(buf, sizeof(buf), "   📝 Pattern utilisé : %s", sch->pattern);
            append_result(buf);
        }
    } else {
        append_text("\n");
        snprintf(buf, sizeof(buf), "❌ RÉSULTAT : NON\n");
        append_text(buf);
        append_text("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");
        
        snprintf(buf, sizeof(buf), "   ✗ Le mot '%s' n'appartient PAS à la racine '%s'\n", 
                 mot, racine_selectionnee);
        append_result(buf);
        append_result("");
        append_result("💡 Suggestions :");
        append_result("   • Vérifiez l'orthographe du mot");
        append_result("   • Essayez une autre racine dans la liste");
        append_result("   • Cliquez sur 'Afficher schèmes' pour voir les patterns disponibles");
    }
    
    g_free(racine_selectionnee);
}

static void on_show_schemes_clicked(GtkButton *btn, gpointer data) {
    (void)btn;
    (void)data;
    
    clear_text();
    append_text("╔═══════════════════════════════════════════════════════════╗\n");
    append_text("║         📐 SCHÈMES MORPHOLOGIQUES DISPONIBLES            ║\n");
    append_text("╚═══════════════════════════════════════════════════════════╝\n\n");
    
    afficherSchemesCallback(ctx.schemes, append_text);
}

static void on_load_schemes_clicked(GtkButton *btn, gpointer data) {
    (void)btn;
    (void)data;
    
    clear_text();
    append_result("📂 Chargement des schèmes...");
    
    int nb = chargerSchemes(ctx.schemes, "schemes.txt");
    if (nb > 0) {
        char buf[256];
        snprintf(buf, sizeof(buf), "✅ %d schème(s) chargé(s) depuis schemes.txt", nb);
        append_result(buf);
    }
}

static void on_stats_tree_clicked(GtkButton *btn, gpointer data) {
    (void)btn;
    (void)data;
    
    clear_text();
    append_text("╔═══════════════════════════════════════════════════════════╗\n");
    append_text("║          📊 STATISTIQUES DE L'ARBRE AVL                  ║\n");
    append_text("╚═══════════════════════════════════════════════════════════╝\n\n");
    
    if (!ctx.racines) {
        append_result("⚠️  Aucune racine chargée.");
        return;
    }
    
    afficherStatistiquesArbre(ctx.racines, append_text);
}

static void on_stats_hash_clicked(GtkButton *btn, gpointer data) {
    (void)btn;
    (void)data;
    
    clear_text();
    append_text("╔═══════════════════════════════════════════════════════════╗\n");
    append_text("║       📊 STATISTIQUES TABLE DE HACHAGE                   ║\n");
    append_text("╚═══════════════════════════════════════════════════════════╝\n\n");
    
    afficherStatistiquesHash(ctx.schemes, append_text);
}

static char* generer_pattern_automatique(const char* nom) {
    static struct {
        const char* nom;
        const char* pattern;
    } patterns[] = {
        {"فاعل", "1ا23"},
        {"مفعول", "م12و3"},
        {"افتعل", "ا1ت23"},
        {"تفعيل", "ت12ي3"},
        {"مفعال", "م12ا3"},
        {"فعيل", "123"},
        {"فعّال", "12ّا3"},
        {"استفعل", "است123"},
        {"انفعل", "ان123"},
        {"تفاعل", "ت1ا23"},
        {"مفاعل", "م1ا23"},
        {"فاعول", "1ا2و3"},
        {NULL, NULL}
    };
    
    for (int i = 0; patterns[i].nom != NULL; i++) {
        if (strcmp(nom, patterns[i].nom) == 0) {
            return g_strdup(patterns[i].pattern);
        }
    }
    
    return g_strdup("123");
}

static void on_targeted_generation_clicked(GtkButton *btn, gpointer data) {
    (void)btn;
    (void)data;
    
    if (!ctx.racines) {
        clear_text();
        append_result("⚠️  Aucune racine chargée");
        append_result("   Cliquez sur '📂 Charger racines' d'abord");
        return;
    }
    
    if (compterSchemes(ctx.schemes) == 0) {
        clear_text();
        append_result("⚠️  Aucun schème disponible");
        return;
    }
    
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        "🎯 Génération morphologique ciblée",
        NULL,
        GTK_DIALOG_MODAL,
        "Annuler", GTK_RESPONSE_CANCEL,
        "Générer", GTK_RESPONSE_OK,
        NULL
    );
    
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_container_set_border_width(GTK_CONTAINER(content_area), 15);
    gtk_widget_set_size_request(content_area, 400, 400);
    
    GtkWidget *label_racine = gtk_label_new("Sélectionnez une racine :");
    gtk_box_pack_start(GTK_BOX(content_area), label_racine, FALSE, FALSE, 5);
    
    GtkWidget *combo_racine = gtk_combo_box_text_new();
    
    int count = compterNoeudsLocal(ctx.racines);
    const char **tab = malloc(count * sizeof(const char *));
    if (tab) {
        int idx = 0;
        remplirTableauLocal(ctx.racines, tab, &idx);
        qsort(tab, count, sizeof(const char *), comparerRacinesLocal);
        
        for (int i = 0; i < count; i++) {
            gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_racine), tab[i]);
        }
        free(tab);
        gtk_combo_box_set_active(GTK_COMBO_BOX(combo_racine), 0);
    }
    
    gtk_box_pack_start(GTK_BOX(content_area), combo_racine, FALSE, FALSE, 5);
    
    GtkWidget *separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start(GTK_BOX(content_area), separator, FALSE, FALSE, 10);
    
    GtkWidget *label_schemes = gtk_label_new("Sélectionnez les schèmes à utiliser :");
    gtk_box_pack_start(GTK_BOX(content_area), label_schemes, FALSE, FALSE, 5);
    
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                   GTK_POLICY_NEVER,
                                   GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(scroll, -1, 200);
    
    GtkWidget *schemes_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
    gtk_container_add(GTK_CONTAINER(scroll), schemes_box);
    gtk_box_pack_start(GTK_BOX(content_area), scroll, TRUE, TRUE, 5);
    
    GtkWidget **checkboxes = malloc(TAILLE_TABLE * sizeof(GtkWidget*));
    char **scheme_names = malloc(TAILLE_TABLE * sizeof(char*));
    int nb_schemes = 0;
    
    for (int i = 0; i < TAILLE_TABLE; i++) {
        EntreeHash* e = ctx.schemes->cases[i];
        while (e) {
            GtkWidget *check = gtk_check_button_new_with_label(e->cle);
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check), TRUE);
            gtk_box_pack_start(GTK_BOX(schemes_box), check, FALSE, FALSE, 2);
            
            checkboxes[nb_schemes] = check;
            scheme_names[nb_schemes] = g_strdup(e->cle);
            nb_schemes++;
            
            e = e->suivant;
        }
    }
    
    gtk_widget_show_all(dialog);
    
    gint result = gtk_dialog_run(GTK_DIALOG(dialog));
    
    if (result == GTK_RESPONSE_OK) {
        gchar *racine = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo_racine));
        
        if (racine) {
            clear_text();
            char buf[512];
            snprintf(buf, sizeof(buf), "🎯 Génération ciblée pour la racine : %s\n", racine);
            append_result(buf);
            append_text("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");
            
            int nb_generes = 0;
            
            for (int i = 0; i < nb_schemes; i++) {
                if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkboxes[i]))) {
                    Scheme* sch = chercherScheme(ctx.schemes, scheme_names[i]);
                    if (sch) {
                        char* mot = genererMot(racine, sch);
                        if (mot) {
                            snprintf(buf, sizeof(buf), "  ✓ %s + %-15s → %s", 
                                     racine, sch->nom, mot);
                            append_result(buf);
                            
                            NoeudArbre* noeud = rechercherRacine(ctx.racines, racine);
                            if (noeud) {
                                ajouterDerive(noeud, mot);
                            }
                            
                            free(mot);
                            nb_generes++;
                        }
                    }
                }
            }
            
            snprintf(buf, sizeof(buf), "\n📊 %d dérivé(s) généré(s) avec les schèmes sélectionnés", nb_generes);
            append_result(buf);
        }
        
        g_free(racine);
    }
    
    for (int i = 0; i < nb_schemes; i++) {
        g_free(scheme_names[i]);
    }
    free(checkboxes);
    free(scheme_names);
    
    gtk_widget_destroy(dialog);
}

static void on_identify_scheme_clicked(GtkButton *btn, gpointer entry) {
    (void)btn;
    
    const char *mot = gtk_entry_get_text(GTK_ENTRY(entry));
    
    if (strlen(mot) == 0) {
        clear_text();
        append_result("⚠️  Veuillez entrer un mot à analyser");
        append_result("");
        append_result("💡 Instructions :");
        append_result("   1. Tapez un mot arabe dans le champ");
        append_result("   2. Cliquez sur '🔍 Identifier schème'");
        return;
    }
    
    gchar *racine_selectionnee = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo_racines));
    
    if (!racine_selectionnee || 
        strcmp(racine_selectionnee, "(Aucune racine - Chargez d'abord)") == 0) {
        clear_text();
        append_result("⚠️  Veuillez sélectionner une racine de référence");
        append_result("   Utilisez la liste déroulante pour choisir");
        g_free(racine_selectionnee);
        return;
    }
    
    clear_text();
    append_text("╔═══════════════════════════════════════════════════════════╗\n");
    append_text("║            🔍 IDENTIFICATION DU SCHÈME                    ║\n");
    append_text("╚═══════════════════════════════════════════════════════════╝\n\n");
    
    char buf[512];
    snprintf(buf, sizeof(buf), "🔍 Analyse du mot : '%s'", mot);
    append_result(buf);
    snprintf(buf, sizeof(buf), "📚 Racine de référence : '%s'\n", racine_selectionnee);
    append_result(buf);
    
    int trouve = 0;
    
    for (int i = 0; i < TAILLE_TABLE; i++) {
        EntreeHash* e = ctx.schemes->cases[i];
        while (e) {
            char* mot_genere = genererMot(racine_selectionnee, &e->valeur);
            
            if (mot_genere) {
                if (strcmp(mot, mot_genere) == 0) {
                    append_text("\n✅ SCHÈME IDENTIFIÉ !\n");
                    append_text("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");
                    
                    snprintf(buf, sizeof(buf), "   📐 Schème : %s", e->valeur.nom);
                    append_result(buf);
                    snprintf(buf, sizeof(buf), "   📝 Pattern : %s", e->valeur.pattern);
                    append_result(buf);
                    snprintf(buf, sizeof(buf), "   ✓ Formule : %s + %s → %s", 
                             racine_selectionnee, e->valeur.nom, mot);
                    append_result(buf);
                    
                    free(mot_genere);
                    trouve = 1;
                    break;
                }
                free(mot_genere);
            }
            e = e->suivant;
        }
        if (trouve) break;
    }
    
    if (!trouve) {
        append_text("\n❌ AUCUN SCHÈME TROUVÉ\n");
        append_text("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");
        snprintf(buf, sizeof(buf), "   Le mot '%s' ne correspond à aucun schème connu", mot);
        append_result(buf);
        snprintf(buf, sizeof(buf), "   avec la racine '%s'", racine_selectionnee);
        append_result(buf);
        append_result("");
        append_result("💡 Suggestions :");
        append_result("   • Vérifiez l'orthographe du mot");
        append_result("   • Essayez une autre racine");
        append_result("   • Le schème utilisé n'est peut-être pas dans la base");
    }
    
    g_free(racine_selectionnee);
}

static void on_add_scheme_clicked(GtkButton *btn, gpointer data) {
    (void)btn;
    (void)data;
    
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        "➕ Ajouter un nouveau schème",
        NULL,
        GTK_DIALOG_MODAL,
        "Annuler", GTK_RESPONSE_CANCEL,
        "Ajouter", GTK_RESPONSE_OK,
        NULL
    );
    
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_container_set_border_width(GTK_CONTAINER(content_area), 15);
    
    GtkWidget *label_info = gtk_label_new("Entrez le nom du schème morphologique :");
    gtk_box_pack_start(GTK_BOX(content_area), label_info, FALSE, FALSE, 5);
    
    GtkWidget *box_nom = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *label_nom = gtk_label_new("Nom du schème :");
    gtk_widget_set_size_request(label_nom, 120, -1);
    GtkWidget *entry_nom = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_nom), "ex: فاعل, مفعول, تفعيل");
    gtk_box_pack_start(GTK_BOX(box_nom), label_nom, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box_nom), entry_nom, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(content_area), box_nom, FALSE, FALSE, 5);
    
    GtkWidget *label_help = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label_help),
        "<small>💡 Le pattern sera généré automatiquement\n"
        "   Schèmes reconnus : فاعل, مفعول, افتعل, تفعيل, etc.</small>");
    gtk_box_pack_start(GTK_BOX(content_area), label_help, FALSE, FALSE, 5);
    
    gtk_widget_show_all(dialog);
    
    gint result = gtk_dialog_run(GTK_DIALOG(dialog));
    
    if (result == GTK_RESPONSE_OK) {
        const char *nom = gtk_entry_get_text(GTK_ENTRY(entry_nom));
        
        if (strlen(nom) > 0) {
            char* pattern = generer_pattern_automatique(nom);
            
            Scheme s = creerScheme(nom, pattern);
            insererScheme(ctx.schemes, s);
            
            clear_text();
            char buf[256];
            snprintf(buf, sizeof(buf), "✅ Schème '%s' ajouté avec succès", nom);
            append_result(buf);
            
            g_free(pattern);
        } else {
            clear_text();
            append_result("❌ Erreur : le nom du schème est obligatoire !");
        }
    }
    
    gtk_widget_destroy(dialog);
}

static void on_edit_scheme_clicked(GtkButton *btn, gpointer data) {
    (void)btn;
    (void)data;
    
    if (compterSchemes(ctx.schemes) == 0) {
        clear_text();
        append_result("⚠️  Aucun schème disponible à modifier");
        append_result("   Chargez d'abord les schèmes ou ajoutez-en un");
        return;
    }
    
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        "✏️ Modifier un schème",
        NULL,
        GTK_DIALOG_MODAL,
        "Annuler", GTK_RESPONSE_CANCEL,
        "Modifier", GTK_RESPONSE_OK,
        NULL
    );
    
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_container_set_border_width(GTK_CONTAINER(content_area), 15);
    
    GtkWidget *label_info = gtk_label_new("Sélectionnez le schème à modifier :");
    gtk_box_pack_start(GTK_BOX(content_area), label_info, FALSE, FALSE, 5);
    
    GtkWidget *box_combo = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *label_combo = gtk_label_new("Schème actuel :");
    gtk_widget_set_size_request(label_combo, 120, -1);
    GtkWidget *combo_schemes = gtk_combo_box_text_new();
    
    for (int i = 0; i < TAILLE_TABLE; i++) {
        EntreeHash* e = ctx.schemes->cases[i];
        while (e) {
            gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_schemes), e->cle);
            e = e->suivant;
        }
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo_schemes), 0);
    
    gtk_box_pack_start(GTK_BOX(box_combo), label_combo, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box_combo), combo_schemes, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(content_area), box_combo, FALSE, FALSE, 5);
    
    GtkWidget *box_nom = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *label_nom = gtk_label_new("Nouveau nom :");
    gtk_widget_set_size_request(label_nom, 120, -1);
    GtkWidget *entry_nom = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_nom), "Entrez le nouveau nom");
    gtk_box_pack_start(GTK_BOX(box_nom), label_nom, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box_nom), entry_nom, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(content_area), box_nom, FALSE, FALSE, 5);
    
    GtkWidget *label_help = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label_help),
        "<small>💡 Le pattern sera regénéré automatiquement</small>");
    gtk_box_pack_start(GTK_BOX(content_area), label_help, FALSE, FALSE, 5);
    
    gtk_widget_show_all(dialog);
    
    gint result = gtk_dialog_run(GTK_DIALOG(dialog));
    
    if (result == GTK_RESPONSE_OK) {
        gchar *ancien_nom = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo_schemes));
        const char *nouveau_nom = gtk_entry_get_text(GTK_ENTRY(entry_nom));
        
        if (ancien_nom && strlen(nouveau_nom) > 0) {
            supprimerScheme(ctx.schemes, ancien_nom);
            
            char* pattern = generer_pattern_automatique(nouveau_nom);
            Scheme s = creerScheme(nouveau_nom, pattern);
            insererScheme(ctx.schemes, s);
            
            clear_text();
            char buf[256];
            snprintf(buf, sizeof(buf), "✅ Schème '%s' remplacé par '%s'", ancien_nom, nouveau_nom);
            append_result(buf);
            
            g_free(pattern);
        } else {
            clear_text();
            append_result("❌ Erreur : le nouveau nom est obligatoire !");
        }
        
        g_free(ancien_nom);
    }
    
    gtk_widget_destroy(dialog);
}

static void on_delete_scheme_clicked(GtkButton *btn, gpointer data) {
    (void)btn;
    (void)data;
    
    if (compterSchemes(ctx.schemes) == 0) {
        clear_text();
        append_result("⚠️  Aucun schème disponible à supprimer");
        return;
    }
    
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        "🗑️ Supprimer un schème",
        NULL,
        GTK_DIALOG_MODAL,
        "Annuler", GTK_RESPONSE_CANCEL,
        "Supprimer", GTK_RESPONSE_OK,
        NULL
    );
    
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_container_set_border_width(GTK_CONTAINER(content_area), 15);
    
    GtkWidget *label_info = gtk_label_new("⚠️ Sélectionnez le schème à supprimer :");
    gtk_box_pack_start(GTK_BOX(content_area), label_info, FALSE, FALSE, 5);
    
    GtkWidget *combo_schemes = gtk_combo_box_text_new();
    
    for (int i = 0; i < TAILLE_TABLE; i++) {
        EntreeHash* e = ctx.schemes->cases[i];
        while (e) {
            gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_schemes), e->cle);
            e = e->suivant;
        }
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo_schemes), 0);
    gtk_box_pack_start(GTK_BOX(content_area), combo_schemes, FALSE, FALSE, 5);
    
    GtkWidget *label_warning = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label_warning),
        "<span color='red'><b>⚠️ Cette action est irréversible !</b></span>");
    gtk_box_pack_start(GTK_BOX(content_area), label_warning, FALSE, FALSE, 10);
    
    gtk_widget_show_all(dialog);
    
    gint result = gtk_dialog_run(GTK_DIALOG(dialog));
    
    if (result == GTK_RESPONSE_OK) {
        gchar *nom = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo_schemes));
        
        if (nom) {
            GtkWidget *confirm = gtk_message_dialog_new(
                NULL,
                GTK_DIALOG_MODAL,
                GTK_MESSAGE_WARNING,
                GTK_BUTTONS_YES_NO,
                "Êtes-vous sûr de vouloir supprimer le schème '%s' ?",
                nom
            );
            
            gint confirm_result = gtk_dialog_run(GTK_DIALOG(confirm));
            gtk_widget_destroy(confirm);
            
            if (confirm_result == GTK_RESPONSE_YES) {
                if (supprimerScheme(ctx.schemes, nom)) {
                    clear_text();
                    char buf[256];
                    snprintf(buf, sizeof(buf), "✅ Schème '%s' supprimé avec succès", nom);
                    append_result(buf);
                } else {
                    clear_text();
                    append_result("❌ Erreur lors de la suppression");
                }
            } else {
                clear_text();
                append_result("❌ Suppression annulée");
            }
        }
        
        g_free(nom);
    }
    
    gtk_widget_destroy(dialog);
}

// ═══════════════════════════════════════════════════════════════════
// NOUVEAU BOUTON 1 : Génération Dynamique
// ═══════════════════════════════════════════════════════════════════

static void on_generation_dynamique_clicked(GtkButton *btn, gpointer data) {
    (void)btn;
    (void)data;
    
    if (!ctx.racines) {
        clear_text();
        append_result("⚠️  Aucune racine chargée !");
        append_result("   Cliquez sur '📂 Charger racines' d'abord");
        return;
    }
    
    if (compterSchemes(ctx.schemes) == 0) {
        clear_text();
        append_result("⚠️  Aucun schème disponible !");
        append_result("   Chargez d'abord les schèmes");
        return;
    }
    
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        "🎲 Génération Dynamique de Dérivés",
        NULL,
        GTK_DIALOG_MODAL,
        "Annuler", GTK_RESPONSE_CANCEL,
        "Générer", GTK_RESPONSE_OK,
        NULL
    );
    
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_container_set_border_width(GTK_CONTAINER(content_area), 20);
    gtk_widget_set_size_request(content_area, 500, 550);
    
    GtkWidget *label_title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label_title),
        "<b><big>🎲 Génération Dynamique</big></b>\n"
        "<small>Sélectionnez racine + schèmes à appliquer</small>");
    gtk_label_set_justify(GTK_LABEL(label_title), GTK_JUSTIFY_CENTER);
    gtk_box_pack_start(GTK_BOX(content_area), label_title, FALSE, FALSE, 10);
    
    GtkWidget *frame_racine = gtk_frame_new("📚 Étape 1 : Choisissez une racine");
    gtk_box_pack_start(GTK_BOX(content_area), frame_racine, FALSE, FALSE, 10);
    
    GtkWidget *box_racine = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_set_border_width(GTK_CONTAINER(box_racine), 10);
    gtk_container_add(GTK_CONTAINER(frame_racine), box_racine);
    
    GtkWidget *combo_racine = gtk_combo_box_text_new();
    
    int count = compterNoeudsLocal(ctx.racines);
    const char **tab = malloc(count * sizeof(const char *));
    if (tab) {
        int idx = 0;
        remplirTableauLocal(ctx.racines, tab, &idx);
        qsort(tab, count, sizeof(const char *), comparerRacinesLocal);
        
        for (int i = 0; i < count; i++) {
            gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_racine), tab[i]);
        }
        free(tab);
        gtk_combo_box_set_active(GTK_COMBO_BOX(combo_racine), 0);
    }
    
    gtk_box_pack_start(GTK_BOX(box_racine), combo_racine, FALSE, FALSE, 5);
    
    GtkWidget *frame_schemes = gtk_frame_new("📐 Étape 2 : Sélectionnez les schèmes");
    gtk_box_pack_start(GTK_BOX(content_area), frame_schemes, TRUE, TRUE, 10);
    
    GtkWidget *box_schemes_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_set_border_width(GTK_CONTAINER(box_schemes_container), 10);
    gtk_container_add(GTK_CONTAINER(frame_schemes), box_schemes_container);
    
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                   GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(scroll, -1, 250);
    gtk_box_pack_start(GTK_BOX(box_schemes_container), scroll, TRUE, TRUE, 5);
    
    GtkWidget *schemes_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
    gtk_container_add(GTK_CONTAINER(scroll), schemes_box);
    
    GtkWidget **checkboxes = malloc(TAILLE_TABLE * sizeof(GtkWidget*));
    char **scheme_names = malloc(TAILLE_TABLE * sizeof(char*));
    int nb_schemes = 0;
    
    for (int i = 0; i < TAILLE_TABLE; i++) {
        EntreeHash* e = ctx.schemes->cases[i];
        while (e) {
            char label[200];
            snprintf(label, sizeof(label), "%s  (%s)", e->cle, e->valeur.pattern);
            
            GtkWidget *check = gtk_check_button_new_with_label(label);
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check), TRUE);
            gtk_box_pack_start(GTK_BOX(schemes_box), check, FALSE, FALSE, 2);
            
            checkboxes[nb_schemes] = check;
            scheme_names[nb_schemes] = g_strdup(e->cle);
            nb_schemes++;
            
            e = e->suivant;
        }
    }
    
    gtk_widget_show_all(dialog);
    gint result = gtk_dialog_run(GTK_DIALOG(dialog));
    
    if (result == GTK_RESPONSE_OK) {
        gchar *racine = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo_racine));
        
        if (racine) {
            clear_text();
            
            append_text("╔═══════════════════════════════════════════════════════════╗\n");
            append_text("║        🎲 GÉNÉRATION DYNAMIQUE DE DÉRIVÉS                ║\n");
            append_text("╚═══════════════════════════════════════════════════════════╝\n\n");
            
            char buf[512];
            snprintf(buf, sizeof(buf), "📌 Racine : %s\n", racine);
            append_result(buf);
            append_text("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");
            
            int nb_generes = 0;
            
            for (int i = 0; i < nb_schemes; i++) {
                if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkboxes[i]))) {
                    Scheme* sch = chercherScheme(ctx.schemes, scheme_names[i]);
                    if (sch) {
                        char* mot = genererMot(racine, sch);
                        if (mot) {
                            snprintf(buf, sizeof(buf), "  ✅ %s + %-15s → %s", 
                                     racine, sch->nom, mot);
                            append_result(buf);
                            
                            NoeudArbre* noeud = rechercherRacine(ctx.racines, racine);
                            if (noeud) {
                                ajouterDerive(noeud, mot);
                            }
                            
                            free(mot);
                            nb_generes++;
                        }
                    }
                }
            }
            
            append_text("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
            snprintf(buf, sizeof(buf), "\n📊 RÉSUMÉ : %d dérivé(s) généré(s)\n", nb_generes);
            append_result(buf);
        }
        
        g_free(racine);
    }
    
    for (int i = 0; i < nb_schemes; i++) {
        g_free(scheme_names[i]);
    }
    free(checkboxes);
    free(scheme_names);
    
    gtk_widget_destroy(dialog);
}

// ═══════════════════════════════════════════════════════════════════
// NOUVEAU BOUTON 2 : Auto-identifier Schème
// ═══════════════════════════════════════════════════════════════════

static void on_auto_identifier_clicked(GtkButton *btn, gpointer data) {
    (void)btn;
    (void)data;
    
    if (!ctx.racines) {
        clear_text();
        append_result("⚠️  Aucune racine chargée !");
        return;
    }
    
    if (compterSchemes(ctx.schemes) == 0) {
        clear_text();
        append_result("⚠️  Aucun schème disponible !");
        return;
    }
    
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        "🔎 Identification Automatique",
        NULL,
        GTK_DIALOG_MODAL,
        "Annuler", GTK_RESPONSE_CANCEL,
        "Analyser", GTK_RESPONSE_OK,
        NULL
    );
    
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_container_set_border_width(GTK_CONTAINER(content_area), 20);
    gtk_widget_set_size_request(content_area, 450, 200);
    
    GtkWidget *label_title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label_title),
        "<b><big>🔎 Identification Automatique</big></b>\n"
        "<small>Trouvez la racine et le schème d'un mot</small>");
    gtk_label_set_justify(GTK_LABEL(label_title), GTK_JUSTIFY_CENTER);
    gtk_box_pack_start(GTK_BOX(content_area), label_title, FALSE, FALSE, 10);
    
    GtkWidget *label_mot = gtk_label_new("Mot arabe à analyser :");
    gtk_box_pack_start(GTK_BOX(content_area), label_mot, FALSE, FALSE, 5);
    
    GtkWidget *entry_mot = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_mot), "ex: كاتب، مكتوب، دارس");
    gtk_widget_set_size_request(entry_mot, -1, 40);
    gtk_box_pack_start(GTK_BOX(content_area), entry_mot, FALSE, FALSE, 5);
    
    GtkWidget *label_info = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label_info),
        "<small><i>💡 Test automatique de toutes les racines et schèmes</i></small>");
    gtk_box_pack_start(GTK_BOX(content_area), label_info, FALSE, FALSE, 10);
    
    gtk_widget_show_all(dialog);
    gint result = gtk_dialog_run(GTK_DIALOG(dialog));
    
    if (result == GTK_RESPONSE_OK) {
        const char *mot = gtk_entry_get_text(GTK_ENTRY(entry_mot));
        
        if (strlen(mot) == 0) {
            clear_text();
            append_result("⚠️  Veuillez entrer un mot !");
            gtk_widget_destroy(dialog);
            return;
        }
        
        clear_text();
        append_text("╔═══════════════════════════════════════════════════════════╗\n");
        append_text("║        🔎 IDENTIFICATION AUTOMATIQUE                     ║\n");
        append_text("╚═══════════════════════════════════════════════════════════╝\n\n");
        
        char buf[512];
        snprintf(buf, sizeof(buf), "🔍 Analyse du mot : '%s'\n", mot);
        append_result(buf);
        append_text("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");
        
        typedef struct {
            char racine[MAX_LEN];
            char scheme[MAX_LEN];
            char pattern[MAX_LEN];
        } Match;
        
        Match *matches = malloc(1000 * sizeof(Match));
        int nb_matches = 0;
        
        void testerRacine(NoeudArbre* n) {
            if (!n) return;
            testerRacine(n->gauche);
            
            for (int i = 0; i < TAILLE_TABLE; i++) {
                EntreeHash* e = ctx.schemes->cases[i];
                while (e) {
                    char* mot_genere = genererMot(n->data.racine, &e->valeur);
                    if (mot_genere) {
                        if (strcmp(mot, mot_genere) == 0) {
                            strcpy(matches[nb_matches].racine, n->data.racine);
                            strcpy(matches[nb_matches].scheme, e->valeur.nom);
                            strcpy(matches[nb_matches].pattern, e->valeur.pattern);
                            nb_matches++;
                        }
                        free(mot_genere);
                    }
                    e = e->suivant;
                }
            }
            
            testerRacine(n->droite);
        }
        
        testerRacine(ctx.racines);
        
        if (nb_matches == 0) {
            append_text("❌ AUCUNE CORRESPONDANCE\n\n");
            append_result("   Le mot ne correspond à aucune combinaison connue.");
            append_result("");
            append_result("💡 Suggestions :");
            append_result("   • Vérifiez l'orthographe");
            append_result("   • Ajoutez la racine manquante");
        } else {
            snprintf(buf, sizeof(buf), "✅ %d CORRESPONDANCE(S) TROUVÉE(S) :\n\n", nb_matches);
            append_text(buf);
            
            for (int i = 0; i < nb_matches; i++) {
                snprintf(buf, sizeof(buf), "┌─ Match #%d ─────────────────────────", i + 1);
                append_result(buf);
                snprintf(buf, sizeof(buf), "│  📚 Racine  : %s", matches[i].racine);
                append_result(buf);
                snprintf(buf, sizeof(buf), "│  📐 Schème  : %s", matches[i].scheme);
                append_result(buf);
                snprintf(buf, sizeof(buf), "│  📝 Pattern : %s", matches[i].pattern);
                append_result(buf);
                snprintf(buf, sizeof(buf), "│  ✓ Formule : %s + %s → %s", 
                         matches[i].racine, matches[i].scheme, mot);
                append_result(buf);
                append_result("└──────────────────────────────────────");
                append_result("");
            }
        }
        
        free(matches);
    }
    
    gtk_widget_destroy(dialog);
}

// ═══════════════════════════════════════════════════════════════════
// NOUVEAU BOUTON 3 : Vérifier Appartenance (mot + racine → OUI/NON)
// ═══════════════════════════════════════════════════════════════════

static void on_verifier_appartenance_clicked(GtkButton *btn, gpointer data) {
    (void)btn;
    (void)data;
    
    if (!ctx.racines) {
        clear_text();
        append_result("⚠️  Aucune racine chargée !");
        return;
    }
    
    if (compterSchemes(ctx.schemes) == 0) {
        clear_text();
        append_result("⚠️  Aucun schème disponible !");
        return;
    }
    
    // Dialogue de saisie
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        "🔍 Vérification d'Appartenance Morphologique",
        NULL,
        GTK_DIALOG_MODAL,
        "Annuler", GTK_RESPONSE_CANCEL,
        "Vérifier", GTK_RESPONSE_OK,
        NULL
    );
    
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_container_set_border_width(GTK_CONTAINER(content_area), 20);
    gtk_widget_set_size_request(content_area, 500, 300);
    
    // En-tête
    GtkWidget *label_title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label_title),
        "<b><big>🔍 Vérification d'Appartenance</big></b>\n"
        "<small>Vérifiez si un mot dérive d'une racine donnée</small>");
    gtk_label_set_justify(GTK_LABEL(label_title), GTK_JUSTIFY_CENTER);
    gtk_box_pack_start(GTK_BOX(content_area), label_title, FALSE, FALSE, 10);
    
    // Champ 1 : Mot à vérifier
    GtkWidget *frame_mot = gtk_frame_new("📝 Étape 1 : Mot dérivé à vérifier");
    gtk_box_pack_start(GTK_BOX(content_area), frame_mot, FALSE, FALSE, 10);
    
    GtkWidget *box_mot = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_set_border_width(GTK_CONTAINER(box_mot), 10);
    gtk_container_add(GTK_CONTAINER(frame_mot), box_mot);
    
    GtkWidget *entry_mot = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_mot), "ex: مكتوب، كاتب، دارس");
    gtk_widget_set_size_request(entry_mot, -1, 40);
    gtk_box_pack_start(GTK_BOX(box_mot), entry_mot, FALSE, FALSE, 5);
    
    // Champ 2 : Racine de référence
    GtkWidget *frame_racine = gtk_frame_new("📚 Étape 2 : Racine de référence");
    gtk_box_pack_start(GTK_BOX(content_area), frame_racine, FALSE, FALSE, 10);
    
    GtkWidget *box_racine = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_set_border_width(GTK_CONTAINER(box_racine), 10);
    gtk_container_add(GTK_CONTAINER(frame_racine), box_racine);
    
    GtkWidget *combo_racine = gtk_combo_box_text_new();
    
    // Remplir avec les racines
    int count = compterNoeudsLocal(ctx.racines);
    const char **tab = malloc(count * sizeof(const char *));
    if (tab) {
        int idx = 0;
        remplirTableauLocal(ctx.racines, tab, &idx);
        qsort(tab, count, sizeof(const char *), comparerRacinesLocal);
        
        for (int i = 0; i < count; i++) {
            gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_racine), tab[i]);
        }
        free(tab);
        gtk_combo_box_set_active(GTK_COMBO_BOX(combo_racine), 0);
    }
    
    gtk_box_pack_start(GTK_BOX(box_racine), combo_racine, FALSE, FALSE, 5);
    
    // Info
    GtkWidget *label_info = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label_info),
        "<small><i>💡 Exemple : كتب + مكتوب → OUI (schème: مفعول)</i></small>");
    gtk_box_pack_start(GTK_BOX(content_area), label_info, FALSE, FALSE, 10);
    
    gtk_widget_show_all(dialog);
    gint result = gtk_dialog_run(GTK_DIALOG(dialog));
    
    // Traitement
    if (result == GTK_RESPONSE_OK) {
        const char *mot = gtk_entry_get_text(GTK_ENTRY(entry_mot));
        gchar *racine = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo_racine));
        
        if (strlen(mot) == 0) {
            clear_text();
            append_result("⚠️  Veuillez entrer un mot à vérifier !");
            g_free(racine);
            gtk_widget_destroy(dialog);
            return;
        }
        
        if (!racine) {
            clear_text();
            append_result("⚠️  Veuillez sélectionner une racine !");
            gtk_widget_destroy(dialog);
            return;
        }
        
        clear_text();
        
        // En-tête des résultats
        append_text("╔═══════════════════════════════════════════════════════════╗\n");
        append_text("║        🔍 VÉRIFICATION D'APPARTENANCE                    ║\n");
        append_text("╚═══════════════════════════════════════════════════════════╝\n\n");
        
        char buf[512];
        snprintf(buf, sizeof(buf), "📝 Mot à vérifier : '%s'", mot);
        append_result(buf);
        snprintf(buf, sizeof(buf), "📚 Racine de référence : '%s'\n", racine);
        append_result(buf);
        append_text("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");
        
        append_result("⏳ Vérification en cours...");
        append_result("   • Décomposition du mot");
        append_result("   • Identification du schème");
        append_result("   • Comparaison avec la racine");
        append_result("");
        
        // Utiliser la fonction de validation
        Scheme *sch_trouve = NULL;
        int appartient = validerMotPourRacine(ctx.racines, ctx.schemes, mot, racine, &sch_trouve);
        
        // Effacer et afficher le résultat final
        clear_text();
        append_text("╔═══════════════════════════════════════════════════════════╗\n");
        append_text("║        🔍 RÉSULTAT DE LA VÉRIFICATION                    ║\n");
        append_text("╚═══════════════════════════════════════════════════════════╝\n\n");
        
        snprintf(buf, sizeof(buf), "📝 Mot : '%s'", mot);
        append_result(buf);
        snprintf(buf, sizeof(buf), "📚 Racine : '%s'\n", racine);
        append_result(buf);
        append_text("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");
        
        if (appartient) {
            // OUI - Le mot appartient à la racine
            append_text("✅ RÉSULTAT : OUI\n\n");
            append_text("╭───────────────────────────────────────────────────────╮\n");
            append_text("│  Le mot appartient bien à cette racine !             │\n");
            append_text("╰───────────────────────────────────────────────────────╯\n\n");
            
            snprintf(buf, sizeof(buf), "   ✓ Le mot '%s' dérive de la racine '%s'", mot, racine);
            append_result(buf);
            
            if (sch_trouve) {
                append_result("");
                append_result("📊 DÉTAILS DE LA DÉRIVATION :");
                append_result("");
                snprintf(buf, sizeof(buf), "   📐 Schème identifié : %s", sch_trouve->nom);
                append_result(buf);
                snprintf(buf, sizeof(buf), "   📝 Pattern utilisé  : %s", sch_trouve->pattern);
                append_result(buf);
                snprintf(buf, sizeof(buf), "   ✓ Formule          : %s + %s → %s", 
                         racine, sch_trouve->nom, mot);
                append_result(buf);
            }
            
            append_result("");
            append_result("💡 Ce mot a été ajouté à la liste des dérivés validés.");
            
        } else {
            // NON - Le mot n'appartient pas à la racine
            append_text("❌ RÉSULTAT : NON\n\n");
            append_text("╭───────────────────────────────────────────────────────╮\n");
            append_text("│  Le mot n'appartient PAS à cette racine              │\n");
            append_text("╰───────────────────────────────────────────────────────╯\n\n");
            
            snprintf(buf, sizeof(buf), "   ✗ Le mot '%s' ne dérive PAS de la racine '%s'", mot, racine);
            append_result(buf);
            
            append_result("");
            append_result("💡 SUGGESTIONS :");
            append_result("");
            append_result("   • Vérifiez l'orthographe du mot");
            append_result("   • Vérifiez l'orthographe de la racine");
            append_result("   • Essayez une autre racine");
            append_result("   • Le schème utilisé n'est peut-être pas dans la base");
            append_result("");
            append_result("   Utilisez le bouton '🔎 Auto-identifier Schème'");
            append_result("   pour trouver automatiquement la bonne racine.");
        }
        
        g_free(racine);
    }
    
    gtk_widget_destroy(dialog);
}

// -----------------------------------------------------------------------------
// Création de l'interface graphique
// -----------------------------------------------------------------------------

static void activate(GtkApplication *app, gpointer user_data) {
    (void)user_data;
    
    // Configuration locale pour support UTF-8 et arabe
    setlocale(LC_ALL, "");
    gtk_init(NULL, NULL);
    
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "🌙 Moteur Morphologique Arabe - القاموس");
    gtk_window_set_default_size(GTK_WINDOW(window), 1200, 800);
    
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(main_box), 10);
    gtk_container_add(GTK_CONTAINER(window), main_box);
    
    GtkWidget *header_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(header_label),
        "<span size='x-large' weight='bold'>🌙 Moteur Morphologique Arabe</span>");
    gtk_box_pack_start(GTK_BOX(main_box), header_label, FALSE, FALSE, 5);
    
    GtkWidget *entry_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(main_box), entry_box, FALSE, FALSE, 0);
    
    GtkWidget *entry_label = gtk_label_new("Mot à tester :");
    gtk_box_pack_start(GTK_BOX(entry_box), entry_label, FALSE, FALSE, 0);
    
    GtkWidget *entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Entrez un mot arabe (ex: مكتوب) ou une racine (ex: كتب)");
    gtk_box_pack_start(GTK_BOX(entry_box), entry, TRUE, TRUE, 0);
    
    GtkWidget *btn_save_root = gtk_button_new_with_label("💾 Enregistrer comme racine");
    gtk_box_pack_start(GTK_BOX(entry_box), btn_save_root, FALSE, FALSE, 0);
    
    GtkWidget *combo_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(main_box), combo_box, FALSE, FALSE, 0);
    
    GtkWidget *combo_label = gtk_label_new("Racine de référence :");
    gtk_box_pack_start(GTK_BOX(combo_box), combo_label, FALSE, FALSE, 0);
    
    combo_racines = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_racines), 
                                   "(Aucune racine - Chargez d'abord)");
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo_racines), 0);
    gtk_box_pack_start(GTK_BOX(combo_box), combo_racines, TRUE, TRUE, 0);
    
    GtkWidget *btn_box1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(main_box), btn_box1, FALSE, FALSE, 0);
    
    GtkWidget *btn_load = gtk_button_new_with_label("📂 Charger racines");
    GtkWidget *btn_show_roots = gtk_button_new_with_label("📚 Afficher racines");
    GtkWidget *btn_generate = gtk_button_new_with_label("🔄 Générer dérivés");
    GtkWidget *btn_validate = gtk_button_new_with_label("✅ Valider mot");
    
    gtk_box_pack_start(GTK_BOX(btn_box1), btn_load, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(btn_box1), btn_show_roots, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(btn_box1), btn_generate, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(btn_box1), btn_validate, TRUE, TRUE, 0);
    
    GtkWidget *btn_box2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(main_box), btn_box2, FALSE, FALSE, 0);
    
    GtkWidget *btn_load_schemes = gtk_button_new_with_label("📂 Charger schèmes");
    GtkWidget *btn_schemes = gtk_button_new_with_label("📐 Afficher schèmes");
    
    gtk_box_pack_start(GTK_BOX(btn_box2), btn_load_schemes, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(btn_box2), btn_schemes, TRUE, TRUE, 0);
    
    GtkWidget *btn_box3 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(main_box), btn_box3, FALSE, FALSE, 0);
    
    GtkWidget *btn_add_scheme = gtk_button_new_with_label("➕ Ajouter schème");
    GtkWidget *btn_edit_scheme = gtk_button_new_with_label("✏️ Modifier schème");
    GtkWidget *btn_delete_scheme = gtk_button_new_with_label("🗑️ Supprimer schème");
    
    gtk_box_pack_start(GTK_BOX(btn_box3), btn_add_scheme, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(btn_box3), btn_edit_scheme, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(btn_box3), btn_delete_scheme, TRUE, TRUE, 0);
    
    // ═══════════════════════════════════════════════════════════════
    // 🎉 LES 3 NOUVEAUX BOUTONS ICI !
    // ═══════════════════════════════════════════════════════════════
    
    GtkWidget *btn_box_nouveaux = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(main_box), btn_box_nouveaux, FALSE, FALSE, 0);
    
    GtkWidget *btn_gen_dynamique = gtk_button_new_with_label("🎲 Génération Dynamique");
    GtkWidget *btn_auto_identify = gtk_button_new_with_label("🔎 Auto-identifier Schème");
    GtkWidget *btn_verifier = gtk_button_new_with_label("🔍 Vérifier Appartenance");
    
    gtk_box_pack_start(GTK_BOX(btn_box_nouveaux), btn_gen_dynamique, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(btn_box_nouveaux), btn_auto_identify, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(btn_box_nouveaux), btn_verifier, TRUE, TRUE, 0);
    
    // ═══════════════════════════════════════════════════════════════
    
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(main_box), scroll, TRUE, TRUE, 0);
    
    result_text = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(result_text), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(result_text), GTK_WRAP_WORD);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(result_text), FALSE);
    
    PangoFontDescription *font_desc = pango_font_description_from_string("Monospace 11");
    gtk_widget_override_font(result_text, font_desc);
    pango_font_description_free(font_desc);
    
    gtk_container_add(GTK_CONTAINER(scroll), result_text);
    
    g_signal_connect(btn_save_root, "clicked", G_CALLBACK(on_save_root_clicked), entry);
    g_signal_connect(btn_load, "clicked", G_CALLBACK(on_load_clicked), NULL);
    g_signal_connect(btn_show_roots, "clicked", G_CALLBACK(on_show_roots_clicked), NULL);
    g_signal_connect(btn_generate, "clicked", G_CALLBACK(on_generate_clicked), entry);
    g_signal_connect(btn_validate, "clicked", G_CALLBACK(on_validate_clicked), entry);
    g_signal_connect(btn_load_schemes, "clicked", G_CALLBACK(on_load_schemes_clicked), NULL);
    g_signal_connect(btn_schemes, "clicked", G_CALLBACK(on_show_schemes_clicked), NULL);
    g_signal_connect(btn_add_scheme, "clicked", G_CALLBACK(on_add_scheme_clicked), NULL);
    g_signal_connect(btn_edit_scheme, "clicked", G_CALLBACK(on_edit_scheme_clicked), NULL);
    g_signal_connect(btn_delete_scheme, "clicked", G_CALLBACK(on_delete_scheme_clicked), NULL);
    
    // ═══════════════════════════════════════════════════════════════
    // 🎉 CONNEXION DES 3 NOUVEAUX BOUTONS !
    // ═══════════════════════════════════════════════════════════════
    
    g_signal_connect(btn_gen_dynamique, "clicked", 
                     G_CALLBACK(on_generation_dynamique_clicked), NULL);
    g_signal_connect(btn_auto_identify, "clicked", 
                     G_CALLBACK(on_auto_identifier_clicked), NULL);
    g_signal_connect(btn_verifier, "clicked", 
                     G_CALLBACK(on_verifier_appartenance_clicked), NULL);
    
    // ═══════════════════════════════════════════════════════════════
    
    append_text("╔═══════════════════════════════════════════════════════════╗\n");
    append_text("║                                                           ║\n");
    append_text("║         🌙 MOTEUR MORPHOLOGIQUE ARABE 🌙                 ║\n");
    append_text("║              Mini-Projet Algorithmique                    ║\n");
    append_text("║                                                           ║\n");
    append_text("╚═══════════════════════════════════════════════════════════╝\n\n");
    append_text("💡 Instructions :\n");
    append_text("   1. Cliquez sur '📂 Charger racines' pour commencer\n");
    append_text("   2. Cliquez sur '📂 Charger schèmes'\n");
    append_text("   3. Testez les 3 NOUVEAUX boutons :\n");
    append_text("      • 🎲 Génération Dynamique\n");
    append_text("      • 🔎 Auto-identifier Schème\n");
    append_text("      • 🔍 Vérifier Appartenance\n\n");
    
    gtk_widget_show_all(window);
}

int lancer_gui(int argc, char *argv[]) {
    GtkApplication *app = gtk_application_new("org.glsi.morpho.arabe",
                                              G_APPLICATION_FLAGS_NONE);
    
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    
    g_object_unref(app);
    
    return status;
}