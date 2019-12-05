#include "stdlib.h"
#include "stdio.h"

int main(int argc, char *argv[]) {
    if(argc < 2) printf("no input files\n");

    FILE *fp = fopen(argv[1],"r");

    while(1) {
        int count;
        if(fread(&count, sizeof count, 1, fp) != 1) break;
        char c;
        if(fread(&c, sizeof c, 1, fp) != 1) break;

        for(int i = 0; i < count; i++)
            fwrite(&c, 1, 1, stdout);
    }
    fclose(fp);
    return(0);
}