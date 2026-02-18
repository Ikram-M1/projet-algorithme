#include <stdio.h>
#include <stdlib.h>
#include "morphologie.h"

int main() {
    const char* racine1 = "بيع"; // b-y-'
    const char* racine2 = "سير"; // s-y-r (may not be in racines.txt)

    Scheme s = creerScheme("افتعل", "ا1ت23");

    char* r1 = genererMotAjwaf(racine1, &s);
    if (r1) {
        printf("%s -> %s\n", racine1, r1);
        free(r1);
    } else {
        printf("Erreur pour %s\n", racine1);
    }

    char* r2 = genererMotAjwaf(racine2, &s);
    if (r2) {
        printf("%s -> %s\n", racine2, r2);
        free(r2);
    } else {
        printf("Erreur pour %s\n", racine2);
    }

    return 0;
}
