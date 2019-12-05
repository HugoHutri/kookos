#include "stdlib.h"
#include "stdio.h"

int main(int argc, char *argv[]) {
    if(argc < 2) printf("no input files\n");

    FILE *fp = fopen(argv[1],"r");

    char c;
    char prev;
    int count = 0;

    while(1) {
        char c = fgetc(fp);

        if(feof(fp)) {
            fwrite(&prev, 1, 1, stdout);
            fwrite(&count, sizeof count, 1, stdout);
            break;
        }
        
        else if(c == prev) {
            prev = c;
            count++;
            continue;
        }

        else if(count != 0) {
            fwrite(&prev, 1, 1, stdout);
            fwrite(&count, sizeof count, 1, stdout);
        }
        count = 1;
        prev = c;
    }
    fclose(fp);
    return(0);
}