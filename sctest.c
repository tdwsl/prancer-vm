#include "scvm.h"
#include <stdio.h>

int main(int argc, char **args) {
    FILE *fp;
    int i;
    char buf[3];

    while(argc > 1 && args[1][0] == '-') {
        if(args[1][1] == 'd') debugEnabled = true;
        else break;
        argc--;
        for(i = 1; i < argc; i++) args[i] = args[i+1];
    }
    if(argc != 2) { printf("usage: %s [-d] <file>\n", args[0]); return 0; }
    fp = fopen(args[1], "rb");
    if(!fp) { printf("failed to open %s\n", args[1]); return 0; }
    fread(buf, 1, 3, fp);
    rpc = buf[0] | ((int)buf[1] << 8) | ((int)buf[2] << 16);
    fread(&memory[rpc], 1, MEMORY_SIZE-rpc, fp);
    rsp = 0;
    fclose(fp);

    while(i = run()) {
        switch(i) {
        case 1: printf("%c", acc); break;
        }
    }

    return 0;
}
