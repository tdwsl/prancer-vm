#include "prvm.h"
#include <stdio.h>
#include <stdbool.h>

int main(int argc, char **args) {
    FILE *fp;
    int i;
    char buf[2];
    bool dump = false;

    while(argc > 1 && args[1][0] == '-') {
        if(args[1][1] == 'd') debugEnabled = true;
        if(args[1][1] == 'm') dump = true;
        else break;
        argc--;
        for(i = 1; i < argc; i++) args[i] = args[i+1];
    }
    if(argc != 2) { printf("usage: %s [-d] <file>\n", args[0]); return 0; }
    fp = fopen(args[1], "rb");
    if(!fp) { printf("failed to open %s\n", args[1]); return 0; }
    fread(buf, 1, 2, fp);
    rpc = buf[0] | ((int)buf[1] << 8);
    fread(&memory[rpc], 1, MEMORY_SIZE-rpc, fp);
    rsp = 0;
    fclose(fp);

    while(i = run()) {
        switch(i) {
        case 1: printf("%c", acc); break;
        case 2: acc = fgetc(stdin); break;
        }
    }

    if(dump) {
        fp = fopen("dump", "w");
        fwrite(memory, 1, 2048, fp);
        fclose(fp);
    }

    return 0;
}
