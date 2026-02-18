# ═══════════════════════════════════════════════════════════════════
# Makefile - Moteur Morphologique Arabe (avec support GTK optionnel)
# ═══════════════════════════════════════════════════════════════════

# Configuration GTK (optionnelle)
GTK_AVAILABLE := $(shell pkg-config --exists gtk+-3.0 && echo yes || echo no)

ifeq ($(GTK_AVAILABLE),yes)
    GTK_CFLAGS = $(shell pkg-config --cflags gtk+-3.0)
    GTK_LIBS   = $(shell pkg-config --libs gtk+-3.0)
    GUI_ENABLED = 1
else
    GTK_CFLAGS =
    GTK_LIBS   =
    GUI_ENABLED = 0
endif

# Compilateur et flags de base
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g -O2
LDFLAGS = -lm

# Ajout des flags GTK si disponible
ifeq ($(GUI_ENABLED),1)
    CFLAGS += $(GTK_CFLAGS) -DGUI_ENABLED
    LDFLAGS += $(GTK_LIBS)
endif

# Nom de l'exécutable
TARGET = morpho_arabe

# Fichiers sources
CORE_SOURCES = arbre.c hash.c morphologie.c
MAIN_SOURCE  = main.c
GUI_SOURCE   = gui.c

# Liste complète des sources selon la disponibilité de GTK
ifeq ($(GUI_ENABLED),1)
    SOURCES = $(CORE_SOURCES) $(MAIN_SOURCE) $(GUI_SOURCE)
else
    SOURCES = $(CORE_SOURCES) $(MAIN_SOURCE)
endif

# Fichiers objets
OBJECTS = $(SOURCES:.c=.o)

# Headers
HEADERS = types.h arbre.h hash.h morphologie.h

# Fichiers de données
DATA_FILES = racines.txt schemes.txt

# ═══════════════════════════════════════════════════════════════════
# Règles principales
# ═══════════════════════════════════════════════════════════════════

# Règle par défaut
all: banner $(TARGET) success

# Bannière de début
banner:
	@echo ""
	@echo "╔═══════════════════════════════════════════════════════════╗"
	@echo "║     🌙 Compilation - Moteur Morphologique Arabe         ║"
	@echo "╚═══════════════════════════════════════════════════════════╝"
	@echo ""
ifeq ($(GUI_ENABLED),1)
	@echo "✅ GTK+-3.0 détecté : Interface graphique activée"
else
	@echo "⚠️  GTK+-3.0 non détecté : Mode console uniquement"
	@echo "   Pour activer la GUI : sudo apt install libgtk-3-dev"
endif
	@echo ""

# Compilation de l'exécutable
$(TARGET): $(OBJECTS)
	@echo "🔗 Liaison de l'exécutable..."
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

# Règle générique pour les fichiers objets
%.o: %.c $(HEADERS)
	@echo "🔨 Compilation de $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# Message de succès
success:
	@echo ""
	@echo "╔═══════════════════════════════════════════════════════════╗"
	@echo "║  ✅ Compilation réussie !                                ║"
	@echo "╚═══════════════════════════════════════════════════════════╝"
	@echo ""
	@echo "Pour lancer le programme :"
	@echo "  ./$(TARGET)"
	@echo ""

# ═══════════════════════════════════════════════════════════════════
# Règles utilitaires
# ═══════════════════════════════════════════════════════════════════

# Nettoie les fichiers objets
clean:
	@echo "🧹 Nettoyage des fichiers objets..."
	@rm -f $(OBJECTS)
	@rm -f *.o
	@echo "✅ Nettoyage terminé"

# Nettoie tout (objets + exécutable)
fclean: clean
	@echo "🗑️  Suppression de l'exécutable..."
	@rm -f $(TARGET)
	@echo "✅ Nettoyage complet terminé"

# Recompile tout depuis zéro
re: fclean all

# Lance le programme après compilation
run: $(TARGET)
	@echo "🚀 Lancement du programme..."
	@echo ""
	./$(TARGET)

# ═══════════════════════════════════════════════════════════════════
# Règles de débogage et tests
# ═══════════════════════════════════════════════════════════════════

# Compilation en mode debug
debug: CFLAGS += -DDEBUG -g3 -O0
debug: fclean $(TARGET)
	@echo ""
	@echo "╔═══════════════════════════════════════════════════════════╗"
	@echo "║  🐛 Version DEBUG compilée                               ║"
	@echo "╚═══════════════════════════════════════════════════════════╝"
	@echo ""

# Compilation optimisée
release: CFLAGS := -Wall -Wextra -std=c11 -O3 -march=native -DNDEBUG
release: fclean $(TARGET)
	@echo ""
	@echo "╔═══════════════════════════════════════════════════════════╗"
	@echo "║  🚀 Version RELEASE compilée                             ║"
	@echo "╚═══════════════════════════════════════════════════════════╝"
	@echo ""

# Analyse statique
check:
	@echo "🔍 Analyse statique du code..."
	@command -v cppcheck >/dev/null 2>&1 && \
		cppcheck --enable=all --suppress=missingIncludeSystem $(SOURCES) || \
		echo "⚠️  cppcheck non installé (optionnel)"

# Test de fuites mémoire
valgrind: $(TARGET)
	@echo "🔍 Test de fuites mémoire..."
	@command -v valgrind >/dev/null 2>&1 && \
		valgrind --leak-check=full ./$(TARGET) || \
		echo "⚠️  valgrind non installé (optionnel)"

# ═══════════════════════════════════════════════════════════════════
# Compilation conditionnelle GUI
# ═══════════════════════════════════════════════════════════════════

# Force GUI (échoue si GTK absent)
gui: 
ifeq ($(GUI_ENABLED),0)
	@echo "❌ GTK+-3.0 non installé"
	@echo "Installation : sudo apt install libgtk-3-dev"
	@exit 1
else
	@$(MAKE) all
endif

# Force mode console uniquement
console: SOURCES = $(CORE_SOURCES) $(MAIN_SOURCE)
console: fclean all
	@echo "✅ Version console compilée"

# ═══════════════════════════════════════════════════════════════════
# Installation
# ═══════════════════════════════════════════════════════════════════

install: $(TARGET)
	@echo "📦 Installation..."
	@sudo cp $(TARGET) /usr/local/bin/
	@sudo chmod 755 /usr/local/bin/$(TARGET)
	@echo "✅ Installation terminée"

uninstall:
	@echo "🗑️  Désinstallation..."
	@sudo rm -f /usr/local/bin/$(TARGET)
	@echo "✅ Terminé"

# Affiche les informations
info:
	@echo ""
	@echo "╔═══════════════════════════════════════════════════════════╗"
	@echo "║     MOTEUR MORPHOLOGIQUE ARABE - Informations            ║"
	@echo "╚═══════════════════════════════════════════════════════════╝"
	@echo ""
	@echo "📂 Sources  : $(SOURCES)"
	@echo "📄 Headers  : $(HEADERS)"
	@echo "🎯 Cible    : $(TARGET)"
ifeq ($(GUI_ENABLED),1)
	@echo "🖥️  GUI      : ✅ Activée (GTK+-3.0)"
else
	@echo "🖥️  GUI      : ❌ Désactivée"
endif
	@echo ""
	@echo "Commandes :"
	@echo "  make        - Compile (auto-détecte GTK)"
	@echo "  make gui    - Force GUI"
	@echo "  make console- Force console"
	@echo "  make run    - Compile et lance"
	@echo "  make clean  - Nettoie .o"
	@echo "  make fclean - Nettoie tout"
	@echo "  make re     - Recompile"
	@echo ""

help: info

# ═══════════════════════════════════════════════════════════════════
# Dépendances
# ═══════════════════════════════════════════════════════════════════

main.o: main.c types.h arbre.h hash.h morphologie.h
arbre.o: arbre.c arbre.h types.h
hash.o: hash.c hash.h types.h
morphologie.o: morphologie.c morphologie.h types.h arbre.h hash.h

ifeq ($(GUI_ENABLED),1)
gui.o: gui.c types.h arbre.h hash.h morphologie.h
endif

.PHONY: all clean fclean re run debug release check valgrind \
        gui console install uninstall info help banner success

.PRECIOUS: %.o