#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "morphologie.h"

void print_hex(const char* s) {
    for (size_t i = 0; i < strlen(s); ++i) {
        printf("%02X ", (unsigned char)s[i]);
    }
    printf("\n");
}

int main() {
    const char* racine = "بيع";
    const char* racine2 = "سير";
    Scheme s = creerScheme("افتعل", "ا1t23"); // note: use ascii 't' for readability? keep original
    // use the actual arabic pattern
    Scheme s2 = creerScheme("افتعل", "ا1ت23");

    char* res = genererMotAjwaf(racine, &s2);
    if (res) {
        printf("racine bytes: "); print_hex(racine);
        printf("result bytes: "); print_hex(res);
        free(res);
    }
    char* res2 = genererMotAjwaf(racine2, &s2);
    if (res2) {
        printf("racine2 bytes: "); print_hex(racine2);
        printf("result2 bytes: "); print_hex(res2);
        free(res2);
    }
    return 0;
}
