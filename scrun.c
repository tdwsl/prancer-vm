#include "scvm.h"
#include <stdio.h>

int main(int argc, char **args) {
    FILE *fp;
    int i;
    char buf[4];

    if(argc != 2) { printf("usage: %s <file>\n", args[0]); return 0; }
    fp = fopen(args[1], "rb");
    if(!fp) { printf("failed to open %s\n", args[1]); return 0; }
    fread(buf, 1, 2, fp);
    rpc = buf[0] | buf[1] << 8;
    fread(&memory[rpc], 1, MEMORY_SIZE-rpc, fp);
    fclose(fp);
    rsp = 0;

    while(i = run()) {
        switch(i) {
        case 1: printf("%c", acc); break;
        }
    }

    return 0;
}
