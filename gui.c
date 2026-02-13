// gui.c — INTERFACE GTK3 AMÉLIORÉE AVEC LISTE DÉROULANTE
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "types.h"
#include "arbre.h"
#include "hash.h"
#include "morphologie.h"

// Contexte global (passé depuis main)
extern Contexte ctx;

// Widgets globaux
static GtkWidget *result_text = NULL;
static GtkWidget *combo_racines = NULL;  // Liste déroulante des racines

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
    
    // Vider la liste existante
    gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(combo_racines));
    
    if (!ctx.racines) {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_racines), 
                                       "(Aucune racine - Chargez d'abord)");
        gtk_combo_box_set_active(GTK_COMBO_BOX(combo_racines), 0);
        return;
    }
    
    // Collecter toutes les racines
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
    
    // Ajouter à la liste déroulante
    for (int i = 0; i < count; i++) {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_racines), tab[i]);
    }
    
    free(tab);
    
    // Sélectionner la première par défaut
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo_racines), 0);
}

// -----------------------------------------------------------------------------
// Callbacks des boutons
// -----------------------------------------------------------------------------

static void on_load_clicked(GtkButton *btn, gpointer data) {
    (void)btn;
    (void)data;
    
    clear_text();
    append_result("📂 Chargement des racines...");
    
    chargerRacinesDepuisFichier(&ctx.racines, "racines.txt");
    append_result("✅ Racines chargées depuis racines.txt");
    
    // Mettre à jour la liste déroulante
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
    
    // Mettre à jour la liste déroulante
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

/**
 * NOUVELLE VERSION : Validation avec mot + sélection de racine via combo
 */
static void on_validate_clicked(GtkButton *btn, gpointer entry) {
    (void)btn;
    
    // Récupérer le mot saisi
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
    
    // Récupérer la racine sélectionnée dans la combo box
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

// -----------------------------------------------------------------------------
// Création de l'interface graphique
// -----------------------------------------------------------------------------

static void activate(GtkApplication *app, gpointer user_data) {
    (void)user_data;
    
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "🌙 Moteur Morphologique Arabe - القاموس");
    gtk_window_set_default_size(GTK_WINDOW(window), 1200, 800);
    
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(main_box), 10);
    gtk_container_add(GTK_CONTAINER(window), main_box);
    
    // En-tête
    GtkWidget *header_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(header_label),
        "<span size='x-large' weight='bold'>🌙 Moteur Morphologique Arabe</span>");
    gtk_box_pack_start(GTK_BOX(main_box), header_label, FALSE, FALSE, 5);
    
    // Zone de saisie mot + bouton enregistrer
    GtkWidget *entry_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(main_box), entry_box, FALSE, FALSE, 0);
    
    GtkWidget *entry_label = gtk_label_new("Mot à tester :");
    gtk_box_pack_start(GTK_BOX(entry_box), entry_label, FALSE, FALSE, 0);
    
    GtkWidget *entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Entrez un mot arabe (ex: مكتوب) ou une racine (ex: كتب)");
    gtk_box_pack_start(GTK_BOX(entry_box), entry, TRUE, TRUE, 0);
    
    GtkWidget *btn_save_root = gtk_button_new_with_label("💾 Enregistrer comme racine");
    gtk_box_pack_start(GTK_BOX(entry_box), btn_save_root, FALSE, FALSE, 0);
    
    // Sélecteur de racine pour validation
    GtkWidget *combo_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(main_box), combo_box, FALSE, FALSE, 0);
    
    GtkWidget *combo_label = gtk_label_new("Racine de référence :");
    gtk_box_pack_start(GTK_BOX(combo_box), combo_label, FALSE, FALSE, 0);
    
    combo_racines = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_racines), 
                                   "(Aucune racine - Chargez d'abord)");
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo_racines), 0);
    gtk_box_pack_start(GTK_BOX(combo_box), combo_racines, TRUE, TRUE, 0);
    
    // Boutons principaux
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
    
    // Boutons secondaires
    GtkWidget *btn_box2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(main_box), btn_box2, FALSE, FALSE, 0);
    
    GtkWidget *btn_load_schemes = gtk_button_new_with_label("📂 Charger schèmes");
    GtkWidget *btn_schemes = gtk_button_new_with_label("📐 Afficher schèmes");
    GtkWidget *btn_stats_tree = gtk_button_new_with_label("📊 Stats Arbre");
    GtkWidget *btn_stats_hash = gtk_button_new_with_label("📊 Stats Hash");
    
    gtk_box_pack_start(GTK_BOX(btn_box2), btn_load_schemes, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(btn_box2), btn_schemes, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(btn_box2), btn_stats_tree, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(btn_box2), btn_stats_hash, TRUE, TRUE, 0);
    
    // Zone de résultats
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
    
    // Connexion des signaux
    g_signal_connect(btn_save_root, "clicked", G_CALLBACK(on_save_root_clicked), entry);
    g_signal_connect(btn_load, "clicked", G_CALLBACK(on_load_clicked), NULL);
    g_signal_connect(btn_show_roots, "clicked", G_CALLBACK(on_show_roots_clicked), NULL);
    g_signal_connect(btn_generate, "clicked", G_CALLBACK(on_generate_clicked), entry);
    g_signal_connect(btn_validate, "clicked", G_CALLBACK(on_validate_clicked), entry);
    g_signal_connect(btn_load_schemes, "clicked", G_CALLBACK(on_load_schemes_clicked), NULL);
    g_signal_connect(btn_schemes, "clicked", G_CALLBACK(on_show_schemes_clicked), NULL);
    g_signal_connect(btn_stats_tree, "clicked", G_CALLBACK(on_stats_tree_clicked), NULL);
    g_signal_connect(btn_stats_hash, "clicked", G_CALLBACK(on_stats_hash_clicked), NULL);
    
    // Message d'accueil
    append_text("╔═══════════════════════════════════════════════════════════╗\n");
    append_text("║                                                           ║\n");
    append_text("║         🌙 MOTEUR MORPHOLOGIQUE ARABE 🌙                 ║\n");
    append_text("║              Mini-Projet Algorithmique                    ║\n");
    append_text("║                                                           ║\n");
    append_text("╚═══════════════════════════════════════════════════════════╝\n\n");
    append_text("💡 Instructions :\n");
    append_text("   1. Cliquez sur '📂 Charger racines' pour commencer\n");
    append_text("   2. La liste déroulante se remplira automatiquement\n");
    append_text("   3. Pour valider un mot :\n");
    append_text("      • Tapez le mot dans le champ\n");
    append_text("      • Sélectionnez une racine dans la liste\n");
    append_text("      • Cliquez sur '✅ Valider mot'\n\n");
    
    gtk_widget_show_all(window);
}

// -----------------------------------------------------------------------------
// Point d'entrée
// -----------------------------------------------------------------------------

int lancer_gui(int argc, char *argv[]) {
    GtkApplication *app = gtk_application_new("org.glsi.morpho.arabe",
                                              G_APPLICATION_FLAGS_NONE);
    
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    
    g_object_unref(app);
    
    return status;
}