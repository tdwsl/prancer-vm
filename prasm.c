#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

enum {
    ARG_NONE=0,
    ARG_WORD,
    ARG_BYTE,
    ARG_REL,
    ARG_REG,
};

struct instruction {
    const char *s;
    unsigned char opc, arg;
};

struct label {
    char s[50];
    uint16_t addr;
};

struct unresolved {
    char s[100];
    char size;
    uint16_t addr;
};

struct fileInfo {
    char name[50];
    size_t line;
};

const struct instruction instructions[] = {
    {"int", 0x00, ARG_BYTE},
    {"b", 0x01, ARG_REL},
    {"bz", 0x02, ARG_REL},
    {"bnz", 0x03, ARG_REL},
    {"bc", 0x04, ARG_REL},
    {"bnc", 0x05, ARG_REL},
    {"shl", 0x08, 0},
    {"shr", 0x09, 0},
    {"inv", 0x0A, 0},
    {"push", 0x0B, 0},
    {"pop", 0x0C, 0},
    {"ret", 0x0D, 0},
    {"call", 0x0E, ARG_WORD},
    {"z", 0x0C, 0},
    {"nz", 0x0D, 0},
    {"c", 0x0E, 0},
    {"nc", 0x0F, 0},
    {"add", 0x80, ARG_REG},
    {"sub", 0x90, ARG_REG},
    {"cmp", 0xA0, ARG_REG},
    {"inc", 0xB0, ARG_REG},
    {"dec", 0xC0, ARG_REG},
    {"and", 0xD0, ARG_REG},
    {"or", 0xE0, ARG_REG},
    {"xor", 0xF0, ARG_REG},
};
const int ninstructions = 24;

char data[65536];
uint16_t size = 0;
uint16_t org = 0;

struct label labels[1000];
struct unresolved unresolved[300];
int nlabels = 0, nunresolved = 0;

struct fileInfo infoStack[120];
int infoSP = 0;

void error(const char *err) {
    printf("error in %s line %lu: %s\n",
            infoStack[infoSP-1].name, infoStack[infoSP-1].line, err);
    exit(1);
}

int findLabel(char *s) {
    int i;
    for(i = 0; i < nlabels; i++)
        if(!strcmp(labels[i].s, s)) return i;
    return -1;
}

int findInstruction(char *s) {
    int i;
    for(i = 0; i < ninstructions; i++)
        if(!strcmp(instructions[i].s, s)) return i;
    return -1;
}

int isHex(char *s, uint16_t *n) {
    if(*s == 0) return 0;
    *n = 0;
    while(*s) {
        if(*s >= '0' && *s <= '9') *n = *n * 16 + *(s++) - '0';
        else if(*s >= 'a' && *s <= 'f') *n = *n * 16 + *(s++) - 'a' + 10;
        else return 0;
    }
    return 1;
}

int isBin(char *s, uint16_t *n) {
    if(*s == 0) return 0;
    *n = 0;
    while(*s) {
        if(*s == '0' || *s == '1') *n = *n * 2 + *(s++) - '0';
        else return 0;
    }
    return 1;
}

int isInteger(char *s, uint16_t *n) {
    char neg;
    if(*s == '$' || *s == '&') return isHex(++s, n);
    if(*s == '%') return isBin(++s, n);
    neg = 0;
    if(*s == '-') { neg = 1; s++; }
    if(*s == 0) return 0;
    *n = 0;
    while(*s) {
        if(*s >= '0' && *s <= '9') *n = *n * 10 + *(s++) - '0';
        else return 0;
    }
    if(neg) *n *= -1;
    return 1;
}

int precedence(char *op) {
    const char *prstr = "+-  |^& */% ~!  ";
    return (int)(strstr(prstr, op)-prstr)/4;
}

int resolveExpression(char *ex, int16_t *v) {
    const char *delim = "()~!/%*|^&-+";
    const char *ops = delim+2;
    char tokheap[1000];
    char *p;
    char *tokens[100];
    char *exarr[100], *dstack[100];
    int istack[100];
    int exsz = 0, dsp = 0, isp = 0;
    int ntokens = 0;
    int i;
    int16_t n;

    p = tokheap;
    while(*ex) {
        while(*ex <= ' ' && *ex) ex++;
        if(strchr(delim, *ex)) {
            while(strchr(delim, *ex))
                { *(p++) = *(ex++); *(p++) = 0; tokens[ntokens++] = p-2; }
        } else {
            i = 0;
            while(!strchr(delim, *ex) && *ex > ' ')
                p[i++] = *(ex++);
            if(i) { p[i++] = 0; tokens[ntokens++] = p; p += i; }
        }
    }

    /*printf("%d\n", ntokens);
    for(i = 0; i < ntokens; i++) printf("[%s]\n", tokens[i]);*/

    if(!ntokens) error("expected expression");

    for(i = 0; i < ntokens; i++)
        if(!strstr(delim, tokens[i]) && !isInteger(tokens[i], &n)
                && findLabel(tokens[i]) == -1)
            return 0;

    if(ntokens == 1) {
        if(strstr(delim, tokens[0])) error("expected value in expression");
        if(isInteger(tokens[0], &n)) *v = n;
        else if((i = findLabel(tokens[0])) != -1) *v = labels[i].addr;
        else return 0;
        return 1;
    }

    for(i = 0; i < ntokens; i++) {
        p = strstr(delim, tokens[i]);
        if(p) {
            if(*p == '(') {
                if(exsz && !strstr(ops, exarr[exsz-1]))
                    error("expected operator before (");
                dstack[dsp++] = 0;
                n = -1;
                while(exsz && strstr(ops, exarr[exsz-1])
                        && precedence(exarr[exsz-1]) > n) {
                    n = precedence(exarr[exsz-1]);
                    dstack[dsp++] = exarr[--exsz];
                }
                exarr[exsz++] = "(";
            } else if(*p == ')') {
                exarr[exsz++] = ")";
                if(!dsp) error("expected ( before )");
                while(dstack[--dsp])
                    exarr[exsz++] = dstack[dsp];
            } else if(exsz && strstr(ops, exarr[exsz-1])
                    && precedence(exarr[exsz-1]) < precedence(tokens[i])) {
                exarr[exsz] = exarr[exsz-1];
                exarr[exsz-1] = tokens[i];
                exsz++;
            } else {
                exarr[exsz++] = tokens[i];
            }
        } else if(exsz && strstr(ops, exarr[exsz-1])) {
            n = -1;
            dstack[dsp++] = 0;
            while(exsz && strstr(ops, exarr[exsz-1])
                    && precedence(exarr[exsz-1]) > n) {
                n = precedence(exarr[exsz-1]);
                dstack[dsp++] = exarr[--exsz];
            }
            exarr[exsz++] = tokens[i];
            while(dstack[--dsp]) exarr[exsz++] = dstack[dsp];
        } else exarr[exsz++] = tokens[i];
        /*for(n = 0; n < exsz; n++) printf("%s ", exarr[n]); printf("\n");*/
    }

    if(dsp) error("expected ) after (");

    for(i = 0; i < exsz; i++) {
        if(strstr(delim, exarr[i])) {
            switch(exarr[i][0]) {
            case '+': istack[--isp-1] += istack[isp]; break;
            case '-': istack[--isp-1] -= istack[isp]; break;
            case '&': istack[--isp-1] &= istack[isp]; break;
            case '^': istack[--isp-1] ^= istack[isp]; break;
            case '|': istack[--isp-1] |= istack[isp]; break;
            case '/': istack[--isp-1] /= istack[isp]; break;
            case '%': istack[--isp-1] %= istack[isp]; break;
            case '*': istack[--isp-1] *= istack[isp]; break;
            case '!': istack[isp-1] = !istack[isp-1]; break;
            case '~': istack[isp-1] = ~istack[isp-1]; break;
            }
        } else if((n = findLabel(exarr[i])) != -1)
            istack[isp++] = labels[n].addr;
        else if(isInteger(exarr[i], &n))
            istack[isp++] = n;
        else error("expression error");
        /*printf("%d\n", istack[isp-1]);*/
    }

    if(isp != 1) error("invalid expression");

    *v = istack[0];
}

int getReg(char *s) {
    int i;
    const char *regStrs[] = {
        "r0", "r1", "r2", "r3",
        "r4", "r5", "r6", "r7",
        "r8", "r9", "r10", "r11",
        "r12", "r13", "r14", "r15",
    };
    for(i = 0; i < 16; i++)
        if(!strcmp(regStrs[i], s)) return i;
    return -1;
}

int indirect(char **s) {
    if(*(*s) == '(' && (*s)[strlen(*s)-1] == ')') {
        (*s)++;
        (*s)[strlen(*s)-1] = 0;
        return 1;
    } else return 0;
}

void addByte(char *ex) {
    int16_t n;
    resolveExpression(ex, &n);
    strcpy(unresolved[nunresolved].s, ex);
    unresolved[nunresolved].addr = size;
    unresolved[nunresolved].size = ARG_BYTE;
    nunresolved++;
    size++;
    org++;
}

void addWord(char *ex) {
    int16_t n;
    resolveExpression(ex, &n);
    strcpy(unresolved[nunresolved].s, ex);
    unresolved[nunresolved].addr = size;
    unresolved[nunresolved].size = ARG_WORD;
    nunresolved++;
    size += 2;
    org += 2;
}

void asmFile(const char *filename);

void opassert(int c) {
    if(!c) error("invalid operand combination");
}

void asmLine(char *line) {
    char strheap[200];
    char *str = strheap, *pstr = strheap;
    char *tokens[300];
    int ntokens = 0;
    char *p;
    int i;
    int16_t n;
    char buf[140];

    /*printf("assembling: %s\n", line);*/

    while(*line && line[strlen(line)-1] <= ' ') line[strlen(line)-1] = 0;
    while(*line && *line > ' ') *(str++) = *(line++);
    *(str++) = 0;
    tokens[ntokens++] = pstr;
    pstr = str;

    if(!strcmp(tokens[0], "include")) {
        while(*line && *line <= ' ') line++;
        if(*line == '"') {
            line++;
            if(line[strlen(line)-1] == '"') line[strlen(line)-1] = 0;
            else error("unterminated string after include");
        }
        asmFile(line);
        return;
    }

    while(*line) {
        while(*line && *line <= ' ') line++;
        if(*line == '"') {
            if(pstr != str) error("unexpected chars before string");
            while(*(++line) != '"' && *line) {
                if(*line == '\\') {
                    switch(*(++line)) {
                    case 'n': n = '\n'; break;
                    case 't': n = '\t'; break;
                    case 'r': n = '\r'; break;
                    case '"': n = '"'; break;
                    default: error("invalid escape character");
                    }
                } else n = *line;
                sprintf(str, "$%.2x", n);
                tokens[ntokens++] = str;
                str += 5;
            }
            pstr = str-5; str--; ntokens--;
            if(*line == 0) error("unterminated string");
            line++;
            while(*line && *line <= ' ') line++;
            if(*line && *line != ',') error("string syntax error");
        } else if(*line == ',') {
            *(str++) = 0;
            tokens[ntokens++] = pstr;
            pstr = str;
            while(*(++line) && *line <= ' ');
        } else
            *(str++) = *(line++);
    }
    *(str++) = 0;
    if(ntokens > 1 || *pstr)
        tokens[ntokens++] = pstr;

    /*for(i = 0; i < ntokens; i++) printf("%s\n", tokens[i]);*/

    if(ntokens == 0) return;

    i = findInstruction(tokens[0]);
    if(i != -1) {
        data[size++] = instructions[i].opc; org++;
        if(instructions[i].arg == ARG_NONE) {
            if(ntokens != 1) error("wrong no of args");
        } else if(ntokens != 2) error("wrong no of args");
        else if(instructions[i].arg == ARG_BYTE) addByte(tokens[1]);
        else if(instructions[i].arg == ARG_REL) {
            sprintf(buf, "(%s)-$%.4x", tokens[1], org+1);
            addByte(buf);
        }
        else if(instructions[i].arg == ARG_WORD) addWord(tokens[1]);
        else if(instructions[i].arg == ARG_REG) {
            i = getReg(tokens[1]);
            if(i == -1) error("invalid register\n");
            data[size-1] |= i;
        }
        else error("instruction parameter error");
        return;
    }

    if(!strcmp(tokens[0], "ld")) {
        if(ntokens != 3) error("wrong no of args for ld");
        if(indirect(&tokens[1])) {
            i = getReg(tokens[1]);
            opassert(!strcmp(tokens[2], "a") && i != -1);
            data[size++] = 0x50 | i; org++;
        } else if(indirect(&tokens[2])) {
            i = getReg(tokens[2]);
            opassert(!strcmp(tokens[1], "a") && i != -1);
            data[size++] = 0x40 | i; org++;
        } else if(!strcmp(tokens[1], "a")) {
            i = getReg(tokens[2]);
            if(i != -1) {
                data[size++] = 0x20 | i; org++;
            } else {
                data[size++] = 0x0F; org++;
                addWord(tokens[2]);
            }
        } else if((i = getReg(tokens[1])) != -1) {
            if(!strcmp(tokens[2], "a")) {
                data[size++] = 0x30 | i; org++;
            } else {
                data[size++] = 0x10 | i; org++;
                addWord(tokens[2]);
            }
        } else opassert(0);
        return;
    }

    if(!strcmp(tokens[0], "ldb")) {
        if(ntokens != 3) error("wrong no of args for ld");
        if(indirect(&tokens[1])) {
            i = getReg(tokens[1]);
            opassert(!strcmp(tokens[2], "a") && i != -1);
            data[size++] = 0x70 | i; org++;
        } else if(indirect(&tokens[2])) {
            i = getReg(tokens[2]);
            opassert(!strcmp(tokens[1], "a") && i != -1);
            data[size++] = 0x60 | i; org++;
        } else opassert(0);
        return;
    }

    if(!strcmp(tokens[0], "org")) {
        if(ntokens != 2) error("wrong no of args for org");
        if(!resolveExpression(tokens[1], &n)) error("failed to resolve org");
        org = n;
        return;
    }

    if(!strcmp(tokens[0], "equ")) {
        if(ntokens != 2) error("wrong no of args for org");
        if(nlabels == 0) error("expected label before equ");
        if(!resolveExpression(tokens[1], &n)) error("failed to resolve equ");
        labels[nlabels-1].addr = n;
        return;
    }

    if(!strcmp(tokens[0], "db")) {
        if(ntokens == 1) error("no args for db");
        for(i = 1; i < ntokens; i++)
            addByte(tokens[i]);
        return;
    }

    if(!strcmp(tokens[0], "dw")) {
        if(ntokens == 1) error("no args for dw");
        for(i = 1; i < ntokens; i++)
            addWord(tokens[i]);
        return;
    }

    if(!strcmp(tokens[0], "resb")) {
        if(ntokens != 3) error("wrong no of args for resb");
        if(!resolveExpression(tokens[1], &n))
            error("failed to resolve quantity for resb");
        for(i = 0; i < n; i++)
            addByte(tokens[2]);
        return;
    }

    if(!strcmp(tokens[0], "resw")) {
        if(ntokens != 3) error("wrong no of args for resw");
        if(!resolveExpression(tokens[1], &n))
            error("failed to resolve quantity for resw");
        for(i = 0; i < n; i++)
            addWord(tokens[2]);
        return;
    }

    if(ntokens != 1) error("unknown instruction");

    strcpy(labels[nlabels].s, tokens[0]);
    labels[nlabels].addr = org;
    nlabels++;
}

void asmFile(const char *filename) {
    FILE *fp;
    char line[500];
    char *p, *p1;
    int i;
    char c;

    fp = fopen(filename, "rb");
    if(!fp) { printf("failed to open %s\n", filename); exit(0); }

    strcpy(infoStack[infoSP].name, filename);
    infoStack[infoSP].line = 1;
    infoSP++;

    while(!feof(fp)) {
        i = 0;
        do
            fread(&line[i++], 1, 1, fp);
        while(line[i-1] != '\n' && !feof(fp));
        line[i-1] = 0;

        p = strchr(line, ';');
        if(p) *p = 0;

        for(p = line; *p; p++) {
            if(*p == '"') while(*(++p) != '"' && *p);
            if(*p >= 'A' && *p <= 'Z') *p += 'a'-'A';
            if(*p == '\t') *p = ' ';
        }

        p = line;
        while(p) {
            while(*p && *p <= ' ') p++;
            if(p1 = strchr(p, ':')) *(p1++) = 0;
            if(*p) asmLine(p);
            p = p1;
        }

        infoStack[infoSP-1].line++;
    }

    infoSP--;

    fclose(fp);
}

int main(int argc, char **args) {
    int i;
    int16_t n;
    FILE *fp;

    if(argc != 3) { printf("usage: %s <file> <out>\n", args[0]); return 0; }
    asmFile(args[1]);

    if(nlabels) {
        printf("labels:\n");
        for(i = 0; i < nlabels; i++)
            printf("$%.4x %s\n", labels[i].addr, labels[i].s);
    }

    for(i = 0; i < nunresolved; i++) {
        if(resolveExpression(unresolved[i].s, &n)) {
            if(unresolved[i].size == ARG_BYTE)
                data[unresolved[i].addr] = n;
            else {
                data[unresolved[i].addr] = n;
                data[unresolved[i].addr+1] = n >> 8;
            }
            unresolved[i--] = unresolved[--nunresolved];
        }
    }

    if(nunresolved) {
        printf("error: unresolved expressions:\n");
        for(i = 0; i < nunresolved; i++)
            printf("  %s\n", unresolved[i].s);
        return 1;
    }

    fp = fopen(args[2], "wb");
    if(!fp) { printf("failed to open %s\n", args[2]); return 0; }
    fwrite(data, 1, size, fp);
    fclose(fp);

    printf("assembled successfully\n");

    return 0;
}
