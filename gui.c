// gui.c — INTERFACE GTK3 MODIFIÉE
// CHANGEMENTS:
// 1. Suppression complète de la barre "Racine de référence"
// 2. Bouton "Valider mot" ouvre une popup avec champ + menu déroulant

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

// -----------------------------------------------------------------------------
// Style CSS Moderne pour l'interface
// -----------------------------------------------------------------------------

static void appliquer_style_moderne(void) {
    GtkCssProvider *provider = gtk_css_provider_new();
   
    const gchar *css_data =
        "/* ═══════════════════════════════════════════════════════════ */\n"
        "/* 🎨 DESIGN COMPACT ET COULEURS PASTEL DOUCES                 */\n"
        "/* ═══════════════════════════════════════════════════════════ */\n"
        "\n"
        "window {\n"
        " background: linear-gradient(145deg, #f5f7fa 0%, #fef5f8 100%);\n"
        "}\n"
        "\n"
        "label#header {\n"
        " color: #5a4a6f;\n"
        " text-shadow: 1px 1px 2px rgba(0,0,0,0.08);\n"
        " padding: 8px 0;\n"
        " font-size: 110%;\n"
        "}\n"
        "\n"
        "entry {\n"
        " border-radius: 6px;\n"
        " border: 1px solid #e0e0e0;\n"
        " padding: 6px 10px;\n"
        " font-size: 13px;\n"
        " background: white;\n"
        " min-height: 32px;\n"
        " box-shadow: inset 0 1px 2px rgba(0,0,0,0.04);\n"
        "}\n"
        "\n"
        "entry:focus {\n"
        " border-color: #b4a7d6;\n"
        " box-shadow: 0 0 0 2px rgba(180,167,214,0.2);\n"
        "}\n"
        "\n"
        "combobox, combobox button {\n"
        " border-radius: 6px;\n"
        " border: 1px solid #e0e0e0;\n"
        " background: white;\n"
        " padding: 5px 10px;\n"
        " min-height: 32px;\n"
        "}\n"
        "\n"
        "button {\n"
        " border: none;\n"
        " border-radius: 6px;\n"
        " padding: 5px 12px;\n"
        " font-weight: 500;\n"
        " font-size: 12px;\n"
        " min-height: 30px;\n"
        " transition: all 0.2s ease;\n"
        " box-shadow: 0 1px 3px rgba(0,0,0,0.08);\n"
        " background: #f5f5f5;\n"
        " color: #4a4a4a;\n"
        "}\n"
        "\n"
        "button:hover {\n"
        " background: #ebebeb;\n"
        " box-shadow: 0 2px 5px rgba(0,0,0,0.12);\n"
        " transform: translateY(-1px);\n"
        "}\n"
        "\n"
        "button:active {\n"
        " background: #e0e0e0;\n"
        " transform: translateY(0);\n"
        "}\n"
        "\n"
        "/* Couleurs pastel douces et apaisantes */\n"
        "button.success {\n"
        " background: linear-gradient(135deg, #d4edda 0%, #c3e6cb 100%);\n"
        " color: #2d5f3f;\n"
        "}\n"
        "button.success:hover {\n"
        " background: linear-gradient(135deg, #c3e6cb 0%, #b1dfbb 100%);\n"
        "}\n"
        "\n"
        "button.warning {\n"
        " background: linear-gradient(135deg, #fff3cd 0%, #ffeaa7 100%);\n"
        " color: #856404;\n"
        "}\n"
        "button.warning:hover {\n"
        " background: linear-gradient(135deg, #ffeaa7 0%, #fdd877 100%);\n"
        "}\n"
        "\n"
        "button.info {\n"
        " background: linear-gradient(135deg, #e7d4f3 0%, #d8bfea 100%);\n"
        " color: #5a2e7a;\n"
        "}\n"
        "button.info:hover {\n"
        " background: linear-gradient(135deg, #d8bfea 0%, #c9aae0 100%);\n"
        "}\n"
        "\n"
        "button.danger {\n"
        " background: linear-gradient(135deg, #f8d7da 0%, #f5c6cb 100%);\n"
        " color: #721c24;\n"
        "}\n"
        "button.danger:hover {\n"
        " background: linear-gradient(135deg, #f5c6cb 0%, #f1b0b7 100%);\n"
        "}\n"
        "\n"
        "textview {\n"
        " border-radius: 8px;\n"
        " padding: 10px;\n"
        " font-family: 'Monospace', 'Courier New';\n"
        " font-size: 12px;\n"
        " background: white;\n"
        " color: #2c2c2c;\n"
        " box-shadow: inset 0 1px 2px rgba(0,0,0,0.04);\n"
        "}\n"
        "\n"
        "scrolledwindow {\n"
        " border-radius: 8px;\n"
        " border: 1px solid #e5e5e5;\n"
        " background: white;\n"
        " box-shadow: 0 2px 6px rgba(0,0,0,0.06);\n"
        "}\n"
        "\n"
        ".section-frame {\n"
        " border: 1px solid #e5e5e5;\n"
        " border-radius: 8px;\n"
        " background: rgba(250,250,250,0.7);\n"
        " padding: 6px;\n"
        " margin: 3px 0;\n"
        "}\n"
        "\n"
        ".section-title {\n"
        " color: #6b6b6b;\n"
        " font-weight: bold;\n"
        " font-size: 95%;\n"
        " margin-bottom: 4px;\n"
        "}\n"
        "\n"
        "* { direction: rtl; text-align: right; }\n";

    gtk_css_provider_load_from_data(provider, css_data, -1, NULL);
    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
    g_object_unref(provider);
}

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

// -----------------------------------------------------------------------------
// Callbacks des boutons
// -----------------------------------------------------------------------------

static void on_load_clicked(GtkButton *btn, gpointer data) {
    (void)btn;
    (void)data;
    
    clear_text();
    // append_result(" Chargement des racines...");
    
    chargerRacinesDepuisFichier(&ctx.racines, "racines.txt");
    append_result(" Racines chargées ");
}

static void on_save_root_clicked(GtkButton *btn, gpointer entry) {
    (void)btn;
    
    const char *racine = gtk_entry_get_text(GTK_ENTRY(entry));
    
    if (strlen(racine) == 0) {
        clear_text();
        append_result("  Veuillez entrer une racine à enregistrer !");
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
}

static void on_show_roots_clicked(GtkButton *btn, gpointer data) {
    (void)btn;
    (void)data;
    
    clear_text();
    
    // En-tête avec design amélioré
    append_text("╔═══════════════════════════════════════════════════════════╗\n");
    append_text("║                                                           ║\n");
    append_text("║           📚  LISTE DES RACINES STOCKÉES  📚              ║\n");
    append_text("║                                                           ║\n");
    append_text("╚═══════════════════════════════════════════════════════════╝\n\n");
    
    if (ctx.racines == NULL) {
        append_text("┌───────────────────────────────────────────────────────────┐\n");
        append_text("│                                                           │\n");
        append_text("│        ⚠️  AUCUNE RACINE CHARGÉE                          │\n");
        append_text("│                                                           │\n");
        append_text("│   💡 Cliquez sur '📂 Charger racines' pour commencer     │\n");
        append_text("│                                                           │\n");
        append_text("└───────────────────────────────────────────────────────────┘\n");
        return;
    }
    
    // Compter les racines
    int total = compterNoeudsLocal(ctx.racines);
    
    // Afficher les statistiques
    char buf[256];
    snprintf(buf, sizeof(buf), "📊 Nombre total de racines : %d\n", total);
    append_result(buf);
    append_text("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");
    
    // Récupérer et trier les racines
    const char **tab = malloc(total * sizeof(const char *));
    if (tab) {
        int idx = 0;
        remplirTableauLocal(ctx.racines, tab, &idx);
        qsort(tab, total, sizeof(const char *), comparerRacinesLocal);
        
        // Afficher les racines en colonnes avec numéros
        append_text("┏━━━━━┳━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓\n");
        append_text("┃  N° ┃  RACINE                                           ┃\n");
        append_text("┡━━━━━╇━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┩\n");
        
        for (int i = 0; i < total; i++) {
            snprintf(buf, sizeof(buf), "│ %3d │  ✦ %-47s│\n", i + 1, tab[i]);
            append_text(buf);
            
            // Ligne de séparation tous les 5 éléments
            if ((i + 1) % 5 == 0 && i + 1 < total) {
                append_text("├─────┼───────────────────────────────────────────────────┤\n");
            }
        }
        
        append_text("└─────┴───────────────────────────────────────────────────┘\n");
        
        free(tab);
    }
    
    append_text("\n");
    append_text("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    snprintf(buf, sizeof(buf), "✅ Affichage terminé - %d racine(s) dans la base\n", total);
    append_result(buf);
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
    
    clear_text();
    append_text("╔═══════════════════════════════════════════════════════════╗\n");
    append_text("║            🔍 IDENTIFICATION DU SCHÈME                    ║\n");
    append_text("╚═══════════════════════════════════════════════════════════╝\n\n");
    
    char buf[512];
    snprintf(buf, sizeof(buf), "🔍 Analyse du mot : '%s'\n", mot);
    append_result(buf);
    
    int trouve = 0;
    
    // Parcourir toutes les racines
    void testerToutesRacines(NoeudArbre* n) {
        if (!n || trouve) return;
        testerToutesRacines(n->gauche);
        
        // Tester tous les schèmes avec cette racine
        for (int i = 0; i < TAILLE_TABLE && !trouve; i++) {
            EntreeHash* e = ctx.schemes->cases[i];
            while (e && !trouve) {
                char* mot_genere = genererMot(n->data.racine, &e->valeur);
                
                if (mot_genere) {
                    if (strcmp(mot, mot_genere) == 0) {
                        append_text("\n✅ SCHÈME IDENTIFIÉ !\n");
                        append_text("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");
                        
                        snprintf(buf, sizeof(buf), "   📚 Racine : %s", n->data.racine);
                        append_result(buf);
                        snprintf(buf, sizeof(buf), "   📐 Schème : %s", e->valeur.nom);
                        append_result(buf);
                        snprintf(buf, sizeof(buf), "   📝 Pattern : %s", e->valeur.pattern);
                        append_result(buf);
                        snprintf(buf, sizeof(buf), "   ✓ Formule : %s + %s → %s", 
                                 n->data.racine, e->valeur.nom, mot);
                        append_result(buf);
                        
                        trouve = 1;
                    }
                    free(mot_genere);
                }
                e = e->suivant;
            }
        }
        
        if (!trouve) testerToutesRacines(n->droite);
    }
    
    if (ctx.racines) {
        testerToutesRacines(ctx.racines);
    }
    
    if (!trouve) {
        append_text("\n❌ AUCUN SCHÈME TROUVÉ\n");
        append_text("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");
        snprintf(buf, sizeof(buf), "   Le mot '%s' ne correspond à aucune combinaison connue", mot);
        append_result(buf);
        append_result("");
        append_result("💡 Suggestions :");
        append_result("   • Vérifiez l'orthographe du mot");
        append_result("   • Chargez plus de racines");
        append_result("   • Ajoutez de nouveaux schèmes");
    }
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
   
    setlocale(LC_ALL, "");
    gtk_init(NULL, NULL);
   
    appliquer_style_moderne();
   
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "🌙 Moteur Morphologique Arabe - القاموس");
    gtk_window_set_default_size(GTK_WINDOW(window), 1350, 920);
   
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_container_set_border_width(GTK_CONTAINER(main_box), 20);
    gtk_container_add(GTK_CONTAINER(window), main_box);
   
    // Header
    GtkWidget *header_label = gtk_label_new(NULL);
    gtk_widget_set_name(header_label, "header");
    gtk_label_set_markup(GTK_LABEL(header_label),
        "<span size='xx-large' weight='bold'>🌙 Moteur Morphologique Arabe</span>\n"
        "<span size='small'>Mini-Projet Algorithmique - GLSI 2025-2026</span>");
    gtk_label_set_justify(GTK_LABEL(header_label), GTK_JUSTIFY_CENTER);
    gtk_box_pack_start(GTK_BOX(main_box), header_label, FALSE, FALSE, 10);
   
    // Entrée mot/racine + bouton save
    GtkWidget *entry_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_box_pack_start(GTK_BOX(main_box), entry_box, FALSE, FALSE, 6);
   
    GtkWidget *entry_label = gtk_label_new("Mot à tester :");
    gtk_box_pack_start(GTK_BOX(entry_box), entry_label, FALSE, FALSE, 0);
   
    GtkWidget *entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Entrez un mot arabe (ex: مكتوب) ou une racine (ex: كتب)");
    gtk_box_pack_start(GTK_BOX(entry_box), entry, TRUE, TRUE, 0);
   
    GtkWidget *btn_save_root = gtk_button_new_with_label("💾 Enregistrer comme racine");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_save_root), "info");
    gtk_box_pack_start(GTK_BOX(entry_box), btn_save_root, FALSE, FALSE, 0);
   
    // SECTION 1 : Chargement et consultation
    GtkWidget *frame1 = gtk_frame_new(NULL);
    gtk_widget_set_name(frame1, "section-frame");
    GtkWidget *box1 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_container_add(GTK_CONTAINER(frame1), box1);
   
    GtkWidget *title1 = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(title1), "<span class='section-title'>Chargement et consultation</span>");
    gtk_box_pack_start(GTK_BOX(box1), title1, FALSE, FALSE, 0);
   
    GtkWidget *row1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(box1), row1, FALSE, FALSE, 0);
   
    GtkWidget *btn_load = gtk_button_new_with_label("📂 Charger racines");
    GtkWidget *btn_show_roots = gtk_button_new_with_label("📚 Afficher racines");
    GtkWidget *btn_load_schemes = gtk_button_new_with_label("📂 Charger schèmes");
    GtkWidget *btn_schemes = gtk_button_new_with_label("📐 Afficher schèmes");
   
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_load), "success");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_show_roots), "info");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_load_schemes), "success");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_schemes), "info");
   
    gtk_box_pack_start(GTK_BOX(row1), btn_load, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(row1), btn_show_roots, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(row1), btn_load_schemes, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(row1), btn_schemes, TRUE, TRUE, 0);
   
    gtk_box_pack_start(GTK_BOX(main_box), frame1, FALSE, FALSE, 8);
   
    // SECTION 2 : Opérations courantes
    GtkWidget *frame2 = gtk_frame_new(NULL);
    gtk_widget_set_name(frame2, "section-frame");
    GtkWidget *box2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_container_add(GTK_CONTAINER(frame2), box2);
   
    GtkWidget *title2 = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(title2), "<span class='section-title'>Opérations sur mot / racine</span>");
    gtk_box_pack_start(GTK_BOX(box2), title2, FALSE, FALSE, 0);
   
    GtkWidget *row2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(box2), row2, FALSE, FALSE, 0);
   
    GtkWidget *btn_generate = gtk_button_new_with_label("🔄 Générer dérivés");
    GtkWidget *btn_validate = gtk_button_new_with_label("✅ Valider mot");
   
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_generate), "warning");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_validate), "success");
   
    gtk_box_pack_start(GTK_BOX(row2), btn_generate, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(row2), btn_validate, TRUE, TRUE, 0);
   
    gtk_box_pack_start(GTK_BOX(main_box), frame2, FALSE, FALSE, 8);
   
    // SECTION 3 : Gestion des schèmes
    GtkWidget *frame3 = gtk_frame_new(NULL);
    gtk_widget_set_name(frame3, "section-frame");
    GtkWidget *box3 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_container_add(GTK_CONTAINER(frame3), box3);
   
    GtkWidget *title3 = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(title3), "<span class='section-title'>Gestion des schèmes</span>");
    gtk_box_pack_start(GTK_BOX(box3), title3, FALSE, FALSE, 0);
   
    GtkWidget *row3 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(box3), row3, FALSE, FALSE, 0);
   
    GtkWidget *btn_add_scheme    = gtk_button_new_with_label("➕ Ajouter schème");
    GtkWidget *btn_edit_scheme   = gtk_button_new_with_label("✏️ Modifier schème");
    GtkWidget *btn_delete_scheme = gtk_button_new_with_label("🗑️ Supprimer schème");
   
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_add_scheme), "success");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_edit_scheme), "warning");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_delete_scheme), "danger");
   
    gtk_box_pack_start(GTK_BOX(row3), btn_add_scheme, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(row3), btn_edit_scheme, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(row3), btn_delete_scheme, TRUE, TRUE, 0);
   
    gtk_box_pack_start(GTK_BOX(main_box), frame3, FALSE, FALSE, 8);
   
    // SECTION 4 : Outils avancés (tes 3 nouveaux boutons)
    GtkWidget *frame4 = gtk_frame_new(NULL);
    gtk_widget_set_name(frame4, "section-frame");
    GtkWidget *box4 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_container_add(GTK_CONTAINER(frame4), box4);
   
    GtkWidget *title4 = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(title4), "<span class='section-title'>Outils avancés</span>");
    gtk_box_pack_start(GTK_BOX(box4), title4, FALSE, FALSE, 0);
   
    GtkWidget *row4 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(box4), row4, FALSE, FALSE, 0);
   
    GtkWidget *btn_gen_dynamique = gtk_button_new_with_label("🎲 Génération Dynamique");
    GtkWidget *btn_auto_identify = gtk_button_new_with_label("🔎 Auto-identifier Schème");
    GtkWidget *btn_verifier      = gtk_button_new_with_label("🔍 Vérifier Appartenance");
   
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_gen_dynamique), "warning");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_auto_identify), "info");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_verifier), "success");
   
    gtk_box_pack_start(GTK_BOX(row4), btn_gen_dynamique, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(row4), btn_auto_identify, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(row4), btn_verifier, TRUE, TRUE, 0);
   
    gtk_box_pack_start(GTK_BOX(main_box), frame4, FALSE, FALSE, 8);
   
    // Zone résultats (plus grande)
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_vexpand(scroll, TRUE);
    gtk_box_pack_end(GTK_BOX(main_box), scroll, TRUE, TRUE, 0);
   
    result_text = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(result_text), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(result_text), GTK_WRAP_WORD);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(result_text), FALSE);
   
    PangoFontDescription *font_desc = pango_font_description_from_string("Monospace 11");
    gtk_widget_override_font(result_text, font_desc);
    pango_font_description_free(font_desc);
   
    gtk_container_add(GTK_CONTAINER(scroll), result_text);
   
    // Connexions
    g_signal_connect(btn_save_root, "clicked", G_CALLBACK(on_save_root_clicked), entry);
    g_signal_connect(btn_load, "clicked", G_CALLBACK(on_load_clicked), NULL);
    g_signal_connect(btn_show_roots, "clicked", G_CALLBACK(on_show_roots_clicked), NULL);
    g_signal_connect(btn_generate, "clicked", G_CALLBACK(on_generate_clicked), entry);
    g_signal_connect(btn_validate, "clicked", G_CALLBACK(on_validate_clicked), NULL);
    g_signal_connect(btn_load_schemes, "clicked", G_CALLBACK(on_load_schemes_clicked), NULL);
    g_signal_connect(btn_schemes, "clicked", G_CALLBACK(on_show_schemes_clicked), NULL);
    g_signal_connect(btn_add_scheme, "clicked", G_CALLBACK(on_add_scheme_clicked), NULL);
    g_signal_connect(btn_edit_scheme, "clicked", G_CALLBACK(on_edit_scheme_clicked), NULL);
    g_signal_connect(btn_delete_scheme, "clicked", G_CALLBACK(on_delete_scheme_clicked), NULL);
   
    g_signal_connect(btn_gen_dynamique, "clicked", G_CALLBACK(on_generation_dynamique_clicked), NULL);
    g_signal_connect(btn_auto_identify, "clicked", G_CALLBACK(on_auto_identifier_clicked), NULL);
    g_signal_connect(btn_verifier, "clicked", G_CALLBACK(on_verifier_appartenance_clicked), NULL);
   
    // Message d'accueil
    append_text("╔═══════════════════════════════════════════════════════════╗\n");
    append_text("║               BIENVENUE DANS LE MOTEUR                   ║\n");
    append_text("╚═══════════════════════════════════════════════════════════╝\n\n");
    append_text("1. Commencez par charger les racines et les schèmes\n");
    append_text("2. Les outils sont regroupés par catégorie\n");
    append_text("3. Le bouton 'Valider mot' ouvre maintenant une popup\n\n");
   
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