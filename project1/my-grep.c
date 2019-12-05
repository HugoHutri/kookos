#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]) {

    if (argc <= 1) {
        perror("")
        exit(1)
    }
    for (int i = 2; i < argc; i++) {
        FILE *tiedosto;
        tiedosto = fopen(argv[i], "r");
        if (tiedosto == NULL) {
            perror("my-grep: cannot open file\n");
            exit(1);
        }

        /* Tämä kasvaa tarvittaessa */
        size_t bufkoko = 100;
        char *buffer = malloc(bufkoko * sizeof(char));

        while( getline(&buffer, &bufkoko, tiedosto) != -1) {
            if (strstr(buffer, argv[1]) != NULL) {
                printf("%s\n", buffer);
            }
        }
        fclose(tiedosto);
    }
    return 0;
} 