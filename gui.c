// gui.c — INTERFACE GTK3 PROFESSIONNELLE ET ÉPURÉE
// VERSION NETTOYÉE - Affichages modernes et minimalistes

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

// Contexte global
extern Contexte ctx;

// Widgets globaux
static GtkWidget *result_text = NULL;

// -----------------------------------------------------------------------------
// Style CSS Moderne
// -----------------------------------------------------------------------------

static void appliquer_style_moderne(void) {
    GtkCssProvider *provider = gtk_css_provider_new();
   
    const gchar *css_data =
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
// Fonctions utilitaires
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
}

static void append_result(const char *text) {
    if (!text) return;
    char *formatted = g_strdup_printf("\u200F%s\u200E\n", text);
    append_text(formatted);
    g_free(formatted);
}

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
// CALLBACKS NETTOYÉS ET PROFESSIONNELS
// -----------------------------------------------------------------------------

static void on_load_clicked(GtkButton *btn, gpointer data) {
    (void)btn;
    (void)data;
    
    clear_text();
    
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(result_text));
    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(buffer, &iter);
    
    GtkTextTag *tag_success = gtk_text_buffer_create_tag(buffer, NULL,
        "foreground", "#10b981",
        "weight", PANGO_WEIGHT_BOLD,
        "scale", 1.2,
        NULL);
    
    chargerRacinesDepuisFichier(&ctx.racines, "racines.txt");
    
    int total = compterNoeudsLocal(ctx.racines);
    char buf[128];
    
    gtk_text_buffer_insert_with_tags(buffer, &iter, "✓ ", -1, tag_success, NULL);
    snprintf(buf, sizeof(buf), "%d racines chargées\n", total);
    gtk_text_buffer_insert_with_tags(buffer, &iter, buf, -1, tag_success, NULL);
}

static void on_save_root_clicked(GtkButton *btn, gpointer entry) {
    (void)btn;
    
    const char *racine = gtk_entry_get_text(GTK_ENTRY(entry));
    
    if (strlen(racine) == 0) {
        clear_text();
        append_result("⚠️ Veuillez entrer une racine");
        return;
    }
    
    size_t len = strlen(racine);
    
    if (len != 6 && len != 8 && len != 10) {
        clear_text();
        append_result("⚠️ Racine invalide (doit être 3 ou 4 lettres arabes)");
        return;
    }
    
    FILE *f = fopen("racines.txt", "a");
    if (!f) {
        clear_text();
        append_result("❌ Erreur d'enregistrement");
        return;
    }
    
    fprintf(f, "%s\n", racine);
    fclose(f);
    
    ctx.racines = insererRacine(ctx.racines, racine);
    
    clear_text();
    
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(result_text));
    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(buffer, &iter);
    
    GtkTextTag *tag_success = gtk_text_buffer_create_tag(buffer, NULL,
        "foreground", "#10b981",
        "weight", PANGO_WEIGHT_BOLD,
        NULL);
    
    char buf[128];
    gtk_text_buffer_insert_with_tags(buffer, &iter, "✓ ", -1, tag_success, NULL);
    snprintf(buf, sizeof(buf), "Racine '%s' enregistrée\n", racine);
    gtk_text_buffer_insert_with_tags(buffer, &iter, buf, -1, tag_success, NULL);
    
    gtk_entry_set_text(GTK_ENTRY(entry), "");
}

static void on_show_roots_clicked(GtkButton *btn, gpointer data) {
    (void)btn;
    (void)data;
    
    clear_text();
    
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(result_text));
    GtkTextIter iter;
    
    GtkTextTag *tag_titre = gtk_text_buffer_create_tag(buffer, NULL,
        "foreground", "#5a4a6f",
        "weight", PANGO_WEIGHT_BOLD,
        "scale", 1.4,
        NULL);
    
    GtkTextTag *tag_ligne = gtk_text_buffer_create_tag(buffer, NULL,
        "foreground", "#9ca3af",
        NULL);
    
    GtkTextTag *tag_numero = gtk_text_buffer_create_tag(buffer, NULL,
        "foreground", "#6c757d",
        "weight", PANGO_WEIGHT_BOLD,
        NULL);
    
    GtkTextTag *tag_racine = gtk_text_buffer_create_tag(buffer, NULL,
        "foreground", "#1a472a",
        "weight", PANGO_WEIGHT_BOLD,
        "scale", 1.2,
        NULL);
    
    GtkTextTag *tag_vert = gtk_text_buffer_create_tag(buffer, NULL,
        "foreground", "#10b981",
        "scale", 1.3,
        NULL);
    
    GtkTextTag *tag_bleu = gtk_text_buffer_create_tag(buffer, NULL,
        "foreground", "#3b82f6",
        "scale", 1.3,
        NULL);
    
    GtkTextTag *tag_jaune = gtk_text_buffer_create_tag(buffer, NULL,
        "foreground", "#f59e0b",
        "scale", 1.3,
        NULL);
    
    GtkTextTag *tag_rouge = gtk_text_buffer_create_tag(buffer, NULL,
        "foreground", "#ef4444",
        "scale", 1.3,
        NULL);
    
    gtk_text_buffer_get_end_iter(buffer, &iter);
    
    // Titre
    gtk_text_buffer_insert(buffer, &iter, "\n  ", -1);
    gtk_text_buffer_insert_with_tags(buffer, &iter, "📚 RACINES STOCKÉES", -1, tag_titre, NULL);
    gtk_text_buffer_insert(buffer, &iter, "\n\n", -1);
    
    if (ctx.racines == NULL) {
        gtk_text_buffer_insert(buffer, &iter, "  ⚠️ Aucune racine chargée\n", -1);
        return;
    }
    
    int total = compterNoeudsLocal(ctx.racines);
    char buf[256];
    
    snprintf(buf, sizeof(buf), "  %d racines\n\n", total);
    gtk_text_buffer_insert(buffer, &iter, buf, -1);
    
    const char **tab = malloc(total * sizeof(const char *));
    if (tab) {
        int idx = 0;
        remplirTableauLocal(ctx.racines, tab, &idx);
        qsort(tab, total, sizeof(const char *), comparerRacinesLocal);
        
        for (int i = 0; i < total; i++) {
            gtk_text_buffer_insert_with_tags(buffer, &iter, "  ┃\n", -1, tag_ligne, NULL);
            gtk_text_buffer_insert_with_tags(buffer, &iter, "  ┣━━ ", -1, tag_ligne, NULL);
            
            GtkTextTag *tag_couleur;
            switch (i % 4) {
                case 0: tag_couleur = tag_vert; break;
                case 1: tag_couleur = tag_bleu; break;
                case 2: tag_couleur = tag_jaune; break;
                default: tag_couleur = tag_rouge; break;
            }
            
            gtk_text_buffer_insert_with_tags(buffer, &iter, "● ", -1, tag_couleur, NULL);
            
            snprintf(buf, sizeof(buf), "%02d. ", i + 1);
            gtk_text_buffer_insert_with_tags(buffer, &iter, buf, -1, tag_numero, NULL);
            
            gtk_text_buffer_insert_with_tags(buffer, &iter, tab[i], -1, tag_racine, NULL);
            gtk_text_buffer_insert(buffer, &iter, "\n", -1);
        }
        
        gtk_text_buffer_insert_with_tags(buffer, &iter, "  ┃\n\n", -1, tag_ligne, NULL);
        
        free(tab);
    }
}

static void on_generate_clicked(GtkButton *btn, gpointer entry) {
    (void)btn;
    
    const char *racine = gtk_entry_get_text(GTK_ENTRY(entry));
    
    if (strlen(racine) == 0) {
        clear_text();
        append_result("⚠️ Entrez une racine");
        return;
    }
    
    clear_text();
    
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(result_text));
    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(buffer, &iter);
    
    GtkTextTag *tag_racine = gtk_text_buffer_create_tag(buffer, NULL,
        "foreground", "#5a4a6f",
        "weight", PANGO_WEIGHT_BOLD,
        "scale", 1.3,
        NULL);
    
    GtkTextTag *tag_derive = gtk_text_buffer_create_tag(buffer, NULL,
        "foreground", "#1a472a",
        "weight", PANGO_WEIGHT_BOLD,
        NULL);
    
    char buf[256];
    
    gtk_text_buffer_insert(buffer, &iter, "\n  ", -1);
    gtk_text_buffer_insert_with_tags(buffer, &iter, "Racine : ", -1, tag_racine, NULL);
    gtk_text_buffer_insert_with_tags(buffer, &iter, racine, -1, tag_racine, NULL);
    gtk_text_buffer_insert(buffer, &iter, "\n\n", -1);
    
    NoeudArbre* noeud = rechercherRacine(ctx.racines, racine);
    if (!noeud) {
        append_result("❌ Racine introuvable");
        return;
    }
    
    int count = 0;
    for (int i = 0; i < TAILLE_TABLE; i++) {
        EntreeHash* e = ctx.schemes->cases[i];
        while (e) {
            char* mot = genererMot(racine, &e->valeur);
            if (mot) {
                ajouterDerive(noeud, mot);
                
                snprintf(buf, sizeof(buf), "  • %s\n", mot);
                gtk_text_buffer_insert_with_tags(buffer, &iter, buf, -1, tag_derive, NULL);
                
                free(mot);
                count++;
            }
            e = e->suivant;
        }
    }
    
    gtk_text_buffer_insert(buffer, &iter, "\n", -1);
    snprintf(buf, sizeof(buf), "  ✓ %d dérivés\n", count);
    gtk_text_buffer_insert(buffer, &iter, buf, -1);
}

static void on_validate_clicked(GtkButton *btn, gpointer data) {
    (void)btn;
    (void)data;
    
    if (!ctx.racines) {
        clear_text();
        append_result("⚠️ Chargez les racines d'abord");
        return;
    }
    
    if (compterSchemes(ctx.schemes) == 0) {
        clear_text();
        append_result("⚠️ Chargez les schèmes d'abord");
        return;
    }
    
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        "✅ Validation",
        NULL,
        GTK_DIALOG_MODAL,
        "Annuler", GTK_RESPONSE_CANCEL,
        "Valider", GTK_RESPONSE_OK,
        NULL
    );
    
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_container_set_border_width(GTK_CONTAINER(content_area), 20);
    gtk_widget_set_size_request(content_area, 450, 250);
    
    GtkWidget *label_mot = gtk_label_new("Mot à valider :");
    gtk_box_pack_start(GTK_BOX(content_area), label_mot, FALSE, FALSE, 5);
    
    GtkWidget *entry_mot = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_mot), "ex: مكتوب");
    gtk_widget_set_size_request(entry_mot, -1, 35);
    gtk_box_pack_start(GTK_BOX(content_area), entry_mot, FALSE, FALSE, 5);
    
    GtkWidget *label_racine = gtk_label_new("Racine de référence :");
    gtk_box_pack_start(GTK_BOX(content_area), label_racine, FALSE, FALSE, 15);
    
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
    
    gtk_widget_show_all(dialog);
    gint result = gtk_dialog_run(GTK_DIALOG(dialog));
    
    if (result == GTK_RESPONSE_OK) {
        const char *mot = gtk_entry_get_text(GTK_ENTRY(entry_mot));
        gchar *racine = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo_racine));
        
        if (strlen(mot) == 0) {
            clear_text();
            append_result("⚠️ Entrez un mot");
            g_free(racine);
            gtk_widget_destroy(dialog);
            return;
        }
        
        clear_text();
        
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(result_text));
        GtkTextIter iter;
        gtk_text_buffer_get_end_iter(buffer, &iter);
        
        GtkTextTag *tag_titre = gtk_text_buffer_create_tag(buffer, NULL,
            "foreground", "#5a4a6f",
            "weight", PANGO_WEIGHT_BOLD,
            "scale", 1.3,
            NULL);
        
        GtkTextTag *tag_success = gtk_text_buffer_create_tag(buffer, NULL,
            "foreground", "#10b981",
            "weight", PANGO_WEIGHT_BOLD,
            "scale", 1.2,
            NULL);
        
        GtkTextTag *tag_error = gtk_text_buffer_create_tag(buffer, NULL,
            "foreground", "#ef4444",
            "weight", PANGO_WEIGHT_BOLD,
            "scale", 1.2,
            NULL);
        
        char buf[256];
        
        gtk_text_buffer_insert(buffer, &iter, "\n  ", -1);
        gtk_text_buffer_insert_with_tags(buffer, &iter, "VALIDATION", -1, tag_titre, NULL);
        gtk_text_buffer_insert(buffer, &iter, "\n\n", -1);
        
        snprintf(buf, sizeof(buf), "  Mot : %s\n", mot);
        gtk_text_buffer_insert(buffer, &iter, buf, -1);
        snprintf(buf, sizeof(buf), "  Racine : %s\n\n", racine);
        gtk_text_buffer_insert(buffer, &iter, buf, -1);
        
        Scheme *sch_trouve = NULL;
        int appartient = validerMotPourRacine(ctx.racines, ctx.schemes, mot, racine, &sch_trouve);
        
        if (appartient) {
            gtk_text_buffer_insert_with_tags(buffer, &iter, "  ✓ OUI", -1, tag_success, NULL);
            gtk_text_buffer_insert(buffer, &iter, " - Le mot appartient à cette racine\n\n", -1);
            
            if (sch_trouve) {
                snprintf(buf, sizeof(buf), "  Schème : %s\n", sch_trouve->nom);
                gtk_text_buffer_insert(buffer, &iter, buf, -1);
                // snprintf(buf, sizeof(buf), "  Pattern : %s\n", sch_trouve->pattern);
                // gtk_text_buffer_insert(buffer, &iter, buf, -1);
            }
        } else {
            gtk_text_buffer_insert_with_tags(buffer, &iter, "  ✗ NON", -1, tag_error, NULL);
            gtk_text_buffer_insert(buffer, &iter, " - Le mot n'appartient pas à cette racine\n", -1);
        }
        
        g_free(racine);
    }
    
    gtk_widget_destroy(dialog);
}

static void on_show_schemes_clicked(GtkButton *btn, gpointer data) {
    (void)btn;
    (void)data;
    
    clear_text();
    
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(result_text));
    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(buffer, &iter);
    
    GtkTextTag *tag_titre = gtk_text_buffer_create_tag(buffer, NULL,
        "foreground", "#5a4a6f",
        "weight", PANGO_WEIGHT_BOLD,
        "scale", 1.4,
        NULL);
    
    GtkTextTag *tag_scheme = gtk_text_buffer_create_tag(buffer, NULL,
        "foreground", "#1a472a",
        "weight", PANGO_WEIGHT_BOLD,
        NULL);
    
    GtkTextTag *tag_pattern = gtk_text_buffer_create_tag(buffer, NULL,
        "foreground", "#6c757d",
        NULL);
    
    gtk_text_buffer_insert(buffer, &iter, "\n  ", -1);
    gtk_text_buffer_insert_with_tags(buffer, &iter, "📐 SCHÈMES DISPONIBLES", -1, tag_titre, NULL);
    gtk_text_buffer_insert(buffer, &iter, "\n\n", -1);
    
    int count = 0;
    char buf[256];
    
    for (int i = 0; i < TAILLE_TABLE; i++) {
        EntreeHash* e = ctx.schemes->cases[i];
        while (e) {
            snprintf(buf, sizeof(buf), "  • ");
            gtk_text_buffer_insert(buffer, &iter, buf, -1);
            
            gtk_text_buffer_insert_with_tags(buffer, &iter, e->valeur.nom, -1, tag_scheme, NULL);
            gtk_text_buffer_insert(buffer, &iter, "  ", -1);
            
            // snprintf(buf, sizeof(buf), "(%s)", e->valeur.pattern);
            // gtk_text_buffer_insert_with_tags(buffer, &iter, buf, -1, tag_pattern, NULL);
            gtk_text_buffer_insert(buffer, &iter, "\n", -1);
            
            count++;
            e = e->suivant;
        }
    }
    
    gtk_text_buffer_insert(buffer, &iter, "\n", -1);
    snprintf(buf, sizeof(buf), "  Total : %d schèmes\n", count);
    gtk_text_buffer_insert(buffer, &iter, buf, -1);
}

static void on_load_schemes_clicked(GtkButton *btn, gpointer data) {
    (void)btn;
    (void)data;
    
    clear_text();
    
    int nb = chargerSchemes(ctx.schemes, "schemes.txt");
    
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(result_text));
    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(buffer, &iter);
    
    GtkTextTag *tag_success = gtk_text_buffer_create_tag(buffer, NULL,
        "foreground", "#10b981",
        "weight", PANGO_WEIGHT_BOLD,
        "scale", 1.2,
        NULL);
    
    char buf[128];
    gtk_text_buffer_insert_with_tags(buffer, &iter, "✓ ", -1, tag_success, NULL);
    snprintf(buf, sizeof(buf), "%d schèmes chargés\n", nb);
    gtk_text_buffer_insert_with_tags(buffer, &iter, buf, -1, tag_success, NULL);
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
        {"فعل", "123"},
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

static void on_add_scheme_clicked(GtkButton *btn, gpointer data) {
    (void)btn;
    (void)data;
    
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        "➕ Nouveau schème",
        NULL,
        GTK_DIALOG_MODAL,
        "Annuler", GTK_RESPONSE_CANCEL,
        "Ajouter", GTK_RESPONSE_OK,
        NULL
    );
    
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_container_set_border_width(GTK_CONTAINER(content_area), 15);
    
    GtkWidget *label_nom = gtk_label_new("Nom du schème :");
    gtk_box_pack_start(GTK_BOX(content_area), label_nom, FALSE, FALSE, 5);
    
    GtkWidget *entry_nom = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_nom), "ex: فاعل");
    gtk_box_pack_start(GTK_BOX(content_area), entry_nom, FALSE, FALSE, 5);
    
    gtk_widget_show_all(dialog);
    gint result = gtk_dialog_run(GTK_DIALOG(dialog));
    
    if (result == GTK_RESPONSE_OK) {
        const char *nom = gtk_entry_get_text(GTK_ENTRY(entry_nom));
        
        if (strlen(nom) > 0) {
            char* pattern = generer_pattern_automatique(nom);
            Scheme s = creerScheme(nom, pattern);
            insererScheme(ctx.schemes, s);
            
            clear_text();
            char buf[128];
            snprintf(buf, sizeof(buf), "✓ Schème '%s' ajouté\n", nom);
            append_result(buf);
            
            g_free(pattern);
        }
    }
    
    gtk_widget_destroy(dialog);
}

static void on_edit_scheme_clicked(GtkButton *btn, gpointer data) {
    (void)btn;
    (void)data;
    
    if (compterSchemes(ctx.schemes) == 0) {
        clear_text();
        append_result("⚠️ Aucun schème disponible");
        return;
    }
    
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        "✏️ Modifier schème",
        NULL,
        GTK_DIALOG_MODAL,
        "Annuler", GTK_RESPONSE_CANCEL,
        "Modifier", GTK_RESPONSE_OK,
        NULL
    );
    
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_container_set_border_width(GTK_CONTAINER(content_area), 15);
    
    GtkWidget *label_actuel = gtk_label_new("Schème actuel :");
    gtk_box_pack_start(GTK_BOX(content_area), label_actuel, FALSE, FALSE, 5);
    
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
    
    GtkWidget *label_nouveau = gtk_label_new("Nouveau nom :");
    gtk_box_pack_start(GTK_BOX(content_area), label_nouveau, FALSE, FALSE, 10);
    
    GtkWidget *entry_nom = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(content_area), entry_nom, FALSE, FALSE, 5);
    
    gtk_widget_show_all(dialog);
    gint result = gtk_dialog_run(GTK_DIALOG(dialog));
    
    if (result == GTK_RESPONSE_OK) {
        gchar *ancien = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo_schemes));
        const char *nouveau = gtk_entry_get_text(GTK_ENTRY(entry_nom));
        
        if (ancien && strlen(nouveau) > 0) {
            supprimerScheme(ctx.schemes, ancien);
            
            char* pattern = generer_pattern_automatique(nouveau);
            Scheme s = creerScheme(nouveau, pattern);
            insererScheme(ctx.schemes, s);
            
            clear_text();
            char buf[128];
            snprintf(buf, sizeof(buf), "✓ Schème modifié\n");
            append_result(buf);
            
            g_free(pattern);
        }
        
        g_free(ancien);
    }
    
    gtk_widget_destroy(dialog);
}

static void on_delete_scheme_clicked(GtkButton *btn, gpointer data) {
    (void)btn;
    (void)data;
    
    if (compterSchemes(ctx.schemes) == 0) {
        clear_text();
        append_result("⚠️ Aucun schème disponible");
        return;
    }
    
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        "🗑️ Supprimer schème",
        NULL,
        GTK_DIALOG_MODAL,
        "Annuler", GTK_RESPONSE_CANCEL,
        "Supprimer", GTK_RESPONSE_OK,
        NULL
    );
    
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_container_set_border_width(GTK_CONTAINER(content_area), 15);
    
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
    
    gtk_widget_show_all(dialog);
    gint result = gtk_dialog_run(GTK_DIALOG(dialog));
    
    if (result == GTK_RESPONSE_OK) {
        gchar *nom = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo_schemes));
        
        if (nom) {
            if (supprimerScheme(ctx.schemes, nom)) {
                clear_text();
                append_result("✓ Schème supprimé\n");
            }
        }
        
        g_free(nom);
    }
    
    gtk_widget_destroy(dialog);
}

static void on_generation_dynamique_clicked(GtkButton *btn, gpointer data) {
    (void)btn;
    (void)data;
    
    if (!ctx.racines || compterSchemes(ctx.schemes) == 0) {
        clear_text();
        append_result("⚠️ Chargez racines et schèmes");
        return;
    }
    
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        "🎲 Génération",
        NULL,
        GTK_DIALOG_MODAL,
        "Annuler", GTK_RESPONSE_CANCEL,
        "Générer", GTK_RESPONSE_OK,
        NULL
    );
    
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_container_set_border_width(GTK_CONTAINER(content_area), 20);
    gtk_widget_set_size_request(content_area, 450, 500);
    
    GtkWidget *label_racine = gtk_label_new("Racine :");
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
    
    GtkWidget *label_schemes = gtk_label_new("Schèmes :");
    gtk_box_pack_start(GTK_BOX(content_area), label_schemes, FALSE, FALSE, 15);
    
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                   GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(scroll, -1, 250);
    
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
            
            GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(result_text));
            GtkTextIter iter;
            gtk_text_buffer_get_end_iter(buffer, &iter);
            
            GtkTextTag *tag_racine_t = gtk_text_buffer_create_tag(buffer, NULL,
                "foreground", "#5a4a6f",
                "weight", PANGO_WEIGHT_BOLD,
                "scale", 1.3,
                NULL);
            
            GtkTextTag *tag_derive = gtk_text_buffer_create_tag(buffer, NULL,
                "foreground", "#1a472a",
                "weight", PANGO_WEIGHT_BOLD,
                NULL);
            
            char buf[256];
            
            gtk_text_buffer_insert(buffer, &iter, "\n  ", -1);
            gtk_text_buffer_insert_with_tags(buffer, &iter, "🎲 GÉNÉRATION", -1, tag_racine_t, NULL);
            gtk_text_buffer_insert(buffer, &iter, "\n\n  Racine : ", -1);
            gtk_text_buffer_insert_with_tags(buffer, &iter, racine, -1, tag_racine_t, NULL);
            gtk_text_buffer_insert(buffer, &iter, "\n\n", -1);
            
            int nb_generes = 0;
            
            for (int i = 0; i < nb_schemes; i++) {
                if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkboxes[i]))) {
                    Scheme* sch = chercherScheme(ctx.schemes, scheme_names[i]);
                    if (sch) {
                        char* mot = genererMot(racine, sch);
                        if (mot) {
                            snprintf(buf, sizeof(buf), "  • %s\n", mot);
                            gtk_text_buffer_insert_with_tags(buffer, &iter, buf, -1, tag_derive, NULL);
                            
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
            
            gtk_text_buffer_insert(buffer, &iter, "\n", -1);
            snprintf(buf, sizeof(buf), "  ✓ %d dérivés\n", nb_generes);
            gtk_text_buffer_insert(buffer, &iter, buf, -1);
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

static void on_auto_identifier_clicked(GtkButton *btn, gpointer data) {
    (void)btn;
    (void)data;
    
    if (!ctx.racines || compterSchemes(ctx.schemes) == 0) {
        clear_text();
        append_result("⚠️ Chargez racines et schèmes");
        return;
    }
    
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        "🔎 Identification",
        NULL,
        GTK_DIALOG_MODAL,
        "Annuler", GTK_RESPONSE_CANCEL,
        "Analyser", GTK_RESPONSE_OK,
        NULL
    );
    
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_container_set_border_width(GTK_CONTAINER(content_area), 20);
    gtk_widget_set_size_request(content_area, 400, 150);
    
    GtkWidget *label_mot = gtk_label_new("Mot à analyser :");
    gtk_box_pack_start(GTK_BOX(content_area), label_mot, FALSE, FALSE, 5);
    
    GtkWidget *entry_mot = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_mot), "ex: كاتب");
    gtk_widget_set_size_request(entry_mot, -1, 35);
    gtk_box_pack_start(GTK_BOX(content_area), entry_mot, FALSE, FALSE, 5);
    
    gtk_widget_show_all(dialog);
    gint result = gtk_dialog_run(GTK_DIALOG(dialog));
    
    if (result == GTK_RESPONSE_OK) {
        const char *mot = gtk_entry_get_text(GTK_ENTRY(entry_mot));
        
        if (strlen(mot) == 0) {
            clear_text();
            append_result("⚠️ Entrez un mot");
            gtk_widget_destroy(dialog);
            return;
        }
        
        clear_text();
        
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(result_text));
        GtkTextIter iter;
        gtk_text_buffer_get_end_iter(buffer, &iter);
        
        GtkTextTag *tag_titre = gtk_text_buffer_create_tag(buffer, NULL,
            "foreground", "#5a4a6f",
            "weight", PANGO_WEIGHT_BOLD,
            "scale", 1.3,
            NULL);
        
        GtkTextTag *tag_info = gtk_text_buffer_create_tag(buffer, NULL,
            "foreground", "#1a472a",
            "weight", PANGO_WEIGHT_BOLD,
            NULL);
        
        char buf[256];
        
        gtk_text_buffer_insert(buffer, &iter, "\n  ", -1);
        gtk_text_buffer_insert_with_tags(buffer, &iter, "🔎 IDENTIFICATION", -1, tag_titre, NULL);
        gtk_text_buffer_insert(buffer, &iter, "\n\n  Mot : ", -1);
        gtk_text_buffer_insert_with_tags(buffer, &iter, mot, -1, tag_titre, NULL);
        gtk_text_buffer_insert(buffer, &iter, "\n\n", -1);
        
        typedef struct {
            char racine[MAX_LEN];
            char scheme[MAX_LEN];
        } Match;
        
        Match *matches = malloc(100 * sizeof(Match));
        int nb_matches = 0;
        
        void testerRacine(NoeudArbre* n) {
            if (!n) return;
            testerRacine(n->gauche);
            
            for (int i = 0; i < TAILLE_TABLE; i++) {
                EntreeHash* e = ctx.schemes->cases[i];
                while (e) {
                    char* mot_genere = genererMot(n->data.racine, &e->valeur);
                    if (mot_genere) {
                        if (strcmp(mot, mot_genere) == 0 && nb_matches < 100) {
                            strcpy(matches[nb_matches].racine, n->data.racine);
                            strcpy(matches[nb_matches].scheme, e->valeur.nom);
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
            gtk_text_buffer_insert(buffer, &iter, "  ❌ Aucune correspondance\n", -1);
        } else {
            for (int i = 0; i < nb_matches; i++) {
                snprintf(buf, sizeof(buf), "  • Racine : ");
                gtk_text_buffer_insert(buffer, &iter, buf, -1);
                gtk_text_buffer_insert_with_tags(buffer, &iter, matches[i].racine, -1, tag_info, NULL);
                
                snprintf(buf, sizeof(buf), "  |  Schème : ");
                gtk_text_buffer_insert(buffer, &iter, buf, -1);
                gtk_text_buffer_insert_with_tags(buffer, &iter, matches[i].scheme, -1, tag_info, NULL);
                gtk_text_buffer_insert(buffer, &iter, "\n", -1);
            }
        }
        
        free(matches);
    }
    
    gtk_widget_destroy(dialog);
}

static void on_verifier_appartenance_clicked(GtkButton *btn, gpointer data) {
    (void)btn;
    (void)data;
    
    // Appelle simplement la validation
    on_validate_clicked(btn, data);
}

// -----------------------------------------------------------------------------
// Interface principale
// -----------------------------------------------------------------------------

static void activate(GtkApplication *app, gpointer user_data) {
    (void)user_data;
   
    setlocale(LC_ALL, "");
    gtk_init(NULL, NULL);
   
    appliquer_style_moderne();
   
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "🌙 Moteur Morphologique Arabe");
    gtk_window_set_default_size(GTK_WINDOW(window), 1350, 920);
   
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_container_set_border_width(GTK_CONTAINER(main_box), 20);
    gtk_container_add(GTK_CONTAINER(window), main_box);
   
    GtkWidget *header_label = gtk_label_new(NULL);
    gtk_widget_set_name(header_label, "header");
    gtk_label_set_markup(GTK_LABEL(header_label),
        "<span size='xx-large' weight='bold'>🌙 Moteur Morphologique Arabe</span>\n"
        );
    gtk_label_set_justify(GTK_LABEL(header_label), GTK_JUSTIFY_CENTER);
    gtk_box_pack_start(GTK_BOX(main_box), header_label, FALSE, FALSE, 10);
   
    GtkWidget *entry_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_box_pack_start(GTK_BOX(main_box), entry_box, FALSE, FALSE, 6);
   
    GtkWidget *entry_label = gtk_label_new("Mot / Racine :");
    gtk_box_pack_start(GTK_BOX(entry_box), entry_label, FALSE, FALSE, 0);
   
    GtkWidget *entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Entrez un mot arabe ou une racine");
    gtk_box_pack_start(GTK_BOX(entry_box), entry, TRUE, TRUE, 0);
   
    GtkWidget *btn_save_root = gtk_button_new_with_label("💾 Enregistrer racine");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_save_root), "info");
    gtk_box_pack_start(GTK_BOX(entry_box), btn_save_root, FALSE, FALSE, 0);
   
    // SECTION 1
    GtkWidget *frame1 = gtk_frame_new(NULL);
    gtk_widget_set_name(frame1, "section-frame");
    GtkWidget *box1 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_container_add(GTK_CONTAINER(frame1), box1);
   
    GtkWidget *title1 = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(title1), "<span class='section-title'>Chargement</span>");
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
   
    // SECTION 2
    GtkWidget *frame2 = gtk_frame_new(NULL);
    gtk_widget_set_name(frame2, "section-frame");
    GtkWidget *box2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_container_add(GTK_CONTAINER(frame2), box2);
   
    GtkWidget *title2 = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(title2), "<span class='section-title'>Opérations</span>");
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
   
    // SECTION 3
    GtkWidget *frame3 = gtk_frame_new(NULL);
    gtk_widget_set_name(frame3, "section-frame");
    GtkWidget *box3 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_container_add(GTK_CONTAINER(frame3), box3);
   
    GtkWidget *title3 = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(title3), "<span class='section-title'>Gestion schèmes</span>");
    gtk_box_pack_start(GTK_BOX(box3), title3, FALSE, FALSE, 0);
   
    GtkWidget *row3 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(box3), row3, FALSE, FALSE, 0);
   
    GtkWidget *btn_add_scheme = gtk_button_new_with_label("➕ Ajouter shèmes");
    GtkWidget *btn_edit_scheme = gtk_button_new_with_label("✏️ Modifier shèmes");
    GtkWidget *btn_delete_scheme = gtk_button_new_with_label("🗑️ Supprimer shèmes");
   
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_add_scheme), "success");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_edit_scheme), "warning");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_delete_scheme), "danger");
   
    gtk_box_pack_start(GTK_BOX(row3), btn_add_scheme, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(row3), btn_edit_scheme, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(row3), btn_delete_scheme, TRUE, TRUE, 0);
   
    gtk_box_pack_start(GTK_BOX(main_box), frame3, FALSE, FALSE, 8);
   
    // SECTION 4
    GtkWidget *frame4 = gtk_frame_new(NULL);
    gtk_widget_set_name(frame4, "section-frame");
    GtkWidget *box4 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_container_add(GTK_CONTAINER(frame4), box4);
   
    GtkWidget *title4 = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(title4), "<span class='section-title'>Outils avancés</span>");
    gtk_box_pack_start(GTK_BOX(box4), title4, FALSE, FALSE, 0);
   
    GtkWidget *row4 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(box4), row4, FALSE, FALSE, 0);
   
    GtkWidget *btn_gen_dynamique = gtk_button_new_with_label("🎲 Génération");
    GtkWidget *btn_auto_identify = gtk_button_new_with_label("🔎 Identifier");
    GtkWidget *btn_verifier = gtk_button_new_with_label("🔍 Vérifier");
   
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_gen_dynamique), "warning");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_auto_identify), "info");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_verifier), "success");
   
    gtk_box_pack_start(GTK_BOX(row4), btn_gen_dynamique, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(row4), btn_auto_identify, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(row4), btn_verifier, TRUE, TRUE, 0);
   
    gtk_box_pack_start(GTK_BOX(main_box), frame4, FALSE, FALSE, 8);
   
    // Zone résultats
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
   
    // Message d'accueil épuré
    append_text("\n\n");
    append_text("  🌙 Bienvenue\n\n");
    append_text("  Commencez par charger les racines et les schèmes\n\n");
   
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