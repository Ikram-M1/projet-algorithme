// // src/gui.c — INTERFACE GRAPHIQUE PROPRE ET CLAIRE
// #include <gtk/gtk.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include "types.h"
// #include "arbre.h"
// #include "hash.h"
// #include "morphologie.h"

// static GtkWidget *text_view;        // Zone de résultats
// static GtkWidget *entry_input;      // Zone de saisie (racine ou mot)
// static GtkWidget *root_list;        // Liste des racines
// static Contexte *ctx_global = NULL;

// // Ajoute du texte dans la zone de résultats
// static void afficher_resultat(const char *texte) {
//     GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
//     GtkTextIter end;
//     gtk_text_buffer_get_end_iter(buffer, &end);
//     gtk_text_buffer_insert(buffer, &end, texte, -1);
//     gtk_text_buffer_insert(buffer, &end, "\n", -1);
// }

// // Bouton : Charger les racines depuis fichier
// static void on_load_clicked(GtkButton *btn, gpointer user_data) {
//     chargerRacinesDepuisFichier(&ctx_global->racines, "racines.txt");
//     afficher_resultat("✓ Racines chargées depuis racines.txt");

//     // Mettre à jour la liste des racines
//     gtk_list_box_remove_all(GTK_LIST_BOX(root_list));
//     // (On remplira la liste plus tard)
// }

// // Bouton : Afficher toutes les racines
// static void on_show_roots_clicked(GtkButton *btn, gpointer user_data) {
//     afficherRacines(ctx_global->racines);
// }

// // Bouton : Générer les dérivés d'une racine
// static void on_generate_clicked(GtkButton *btn, gpointer user_data) {
//     const char *racine = gtk_entry_get_text(GTK_ENTRY(entry_input));
//     if (strlen(racine) == 0) {
//         afficher_resultat("⚠ Entrez une racine d'abord");
//         return;
//     }

//     afficher_resultat("=== Génération des dérivés ===");
//     genererFamilleMorphologique(ctx_global->racines, ctx_global->schemes, racine);
//     afficher_resultat("Génération terminée.");
// }

// // Bouton : Valider un mot pour une racine
// static void on_validate_clicked(GtkButton *btn, gpointer user_data) {
//     const char *input = gtk_entry_get_text(GTK_ENTRY(entry_input));
//     // Pour simplifier : on suppose que l'utilisateur entre "racine mot" séparés par espace
//     char racine[64], mot[64];
//     if (sscanf(input, "%s %s", racine, mot) != 2) {
//         afficher_resultat("Format : racine mot");
//         return;
//     }

//     Scheme *sch = NULL;
//     int ok = validerMotPourRacine(ctx_global->racines, ctx_global->schemes, mot, racine, &sch);

//     if (ok) {
//         afficher_resultat("✓ OUI - Le mot appartient à la racine");
//         if (sch) {
//             char buf[200];
//             snprintf(buf, sizeof(buf), "   Schème reconnu : %s", sch->nom);
//             afficher_resultat(buf);
//         }
//     } else {
//         afficher_resultat("✗ NON - Le mot n'appartient pas à cette racine");
//     }
// }

// // Bouton : Afficher les schèmes
// static void on_show_schemes_clicked(GtkButton *btn, gpointer user_data) {
//     afficher_resultat("=== Schèmes disponibles ===");
//     afficherSchemes(ctx_global->schemes);
// }

// // Création de l'interface
// void launch_gui(int argc, char *argv[], Contexte *ctx) {
//     ctx_global = ctx;

//     gtk_init(&argc, &argv);

//     GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
//     gtk_window_set_title(GTK_WINDOW(window), "Moteur Morphologique Arabe");
//     gtk_window_set_default_size(GTK_WINDOW(window), 1200, 800);
//     g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

//     GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
//     gtk_container_add(GTK_CONTAINER(window), box);

//     // === Barre de boutons ===
//     GtkWidget *btn_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
//     gtk_box_pack_start(GTK_BOX(box), btn_box, FALSE, FALSE, 10);

//     GtkWidget *btn_load = gtk_button_new_with_label("Charger racines");
//     GtkWidget *btn_show_roots = gtk_button_new_with_label("Afficher racines");
//     GtkWidget *btn_generate = gtk_button_new_with_label("Générer dérivés");
//     GtkWidget *btn_validate = gtk_button_new_with_label("Valider mot");
//     GtkWidget *btn_show_schemes = gtk_button_new_with_label("Afficher schèmes");

//     gtk_box_pack_start(GTK_BOX(btn_box), btn_load, TRUE, TRUE, 0);
//     gtk_box_pack_start(GTK_BOX(btn_box), btn_show_roots, TRUE, TRUE, 0);
//     gtk_box_pack_start(GTK_BOX(btn_box), btn_generate, TRUE, TRUE, 0);
//     gtk_box_pack_start(GTK_BOX(btn_box), btn_validate, TRUE, TRUE, 0);
//     gtk_box_pack_start(GTK_BOX(btn_box), btn_show_schemes, TRUE, TRUE, 0);

//     g_signal_connect(btn_load, "clicked", G_CALLBACK(on_load_clicked), NULL);
//     g_signal_connect(btn_show_roots, "clicked", G_CALLBACK(on_show_roots_clicked), NULL);
//     g_signal_connect(btn_generate, "clicked", G_CALLBACK(on_generate_clicked), NULL);
//     g_signal_connect(btn_validate, "clicked", G_CALLBACK(on_validate_clicked), NULL);
//     g_signal_connect(btn_show_schemes, "clicked", G_CALLBACK(on_show_schemes_clicked), NULL);

//     // Zone de saisie
//     entry_input = gtk_entry_new();
//     gtk_entry_set_placeholder_text(GTK_ENTRY(entry_input), "Exemple : كتب  ou  كتب مكتوب");
//     gtk_box_pack_start(GTK_BOX(box), entry_input, FALSE, FALSE, 5);

//     // Zone de résultats
//     GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
//     text_view = gtk_text_view_new();
//     gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
//     gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD);
//     gtk_container_add(GTK_CONTAINER(scroll), text_view);
//     gtk_box_pack_start(GTK_BOX(box), scroll, TRUE, TRUE, 0);

//     gtk_widget_show_all(window);
//     gtk_main();
// }