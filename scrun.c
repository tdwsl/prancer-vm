#include "qcvm.h"
#include <stdio.h>

int main(int argc, char **args) {
    FILE *fp;
    int i;

    if(argc != 2) { printf("usage: %s <file>\n", args[0]); return 0; }
    fp = fopen(args[1], "rb");
    if(!fp) { printf("failed to open %s\n", args[1]); return 0; }
    fread(&pc, 1, 2, fp);
    pc = little(pc);
    fread(&memory[pc], 1, 65536, fp);
    fclose(fp);

    while(i = run()) {
        switch(i) {
        case 1: printf("%c", a); break;
        }
    }

    return 0;
}
