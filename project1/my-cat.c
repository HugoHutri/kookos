#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {

    for (int i = 1; i <= argc-1; i++) {
        char muisti[200];
        FILE *tiedosto;
        tiedosto = fopen(argv[i], "r");
        if (tiedosto == NULL) {
            perror("my-cat: cannot open file\n");
            exit(0);
        }

        while(fgets(muisti, 199, tiedosto) != NULL) {
            printf("%s\n", muisti);
        }
        fclose(tiedosto);
    }
    return 0;
} 