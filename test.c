#include <stdio.h>
#include <windows.h>

int main(void) {
    printf("Test 1: avant UTF-8\n");
    fflush(stdout);
    
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
    
    printf("Test 2: après UTF-8\n");
    printf("Bonjour avec accents: café, naïf\n");
    
    return 0;
}
