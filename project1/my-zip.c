#include "stdlib.h"
#include "stdio.h"

int main(int argc, char *argv[]) {
    if(argc < 2) printf("no input files\n");

    for(int i = 1; i < argc; i++)
    {
        FILE *fp = fopen(argv[i],"r");
        if(fp == NULL) {
            printf("Tiedoston %s avaaminen epäonnistui\n", argv[i]);
            continue;
        }

        char c;
        char prev;
        int count = 0;

        while(1) {
            char c = fgetc(fp);

            if(feof(fp)) {
                fwrite(&count, sizeof count, 1, stdout);
                fwrite(&prev, 1, 1, stdout);
                break;
            }
            
            else if(c == prev) {
                prev = c;
                count++;
                continue;
            }

            else if(count != 0) {
                fwrite(&count, sizeof count, 1, stdout);
                fwrite(&prev, 1, 1, stdout);
            }
            count = 1;
            prev = c;
        }
        fclose(fp);
    }
    return(0);
}