#include "qcvm.h"
#include <stdint.h>
#include <stdbool.h>

unsigned char memory[65536] = {0};
unsigned char sp = 0;
uint16_t pc = 0;
uint16_t regs[16] = {0};
uint16_t a;
bool zf = 0, cf = 0;

int insSize(unsigned char ins) {
    if(ins == 0) return 2;
    if(ins & 0x10 == 0x10) return 3;
    if(ins & 0x08 == 0) return 1;
    if(ins & 0x04 == 0) return 2;
    return 1;
}

int run() {
    for(;;) {
        printf("%.4x %.2x\n", pc, memory[pc]);
        switch(memory[pc] & 0x10) {
        case 0x00:
            switch(memory[pc]) {
            case 0x00:
                pc += 2;
                return memory[pc-1];
            case 0x01:
                cf = a & 0x8000;
                a <<= 1;
                zf = !a;
                break;
            case 0x02:
                cf = a & 1;
                a >>= 1;
                zf = !a;
                break;
            case 0x03:
                a = ~a;
                zf = !a;
                break;
            case 0x04:
                *(uint16_t*)&memory[sp] = little(a);
                sp += 2;
                break;
            case 0x05:
                sp -= 2;
                a = little(*(uint16_t*)&memory[sp]);
                break;
            case 0x06:
                sp -= 2;
                pc = little(*(uint16_t*)&memory[sp]);
                break;
            case 0x07:
                if(a & 0x80) a |= 0xff00;
                else a &= 0x00ff;
                zf = !a;
                break;
            case 0x08:
                *(uint16_t*)&memory[sp] = little(pc);
                sp += 2;
                pc = little(*(uint16_t*)&memory[pc+1]);
                continue;
            case 0x09:
                pc = little(*(uint16_t*)&memory[pc+1]);
                break;
            case 0x0A:
                a = little(*(uint16_t*)&memory[pc+1]);
                break;
            case 0x0C:
                if(!zf) pc++;
                break;
            case 0x0D:
                if(zf) pc++;
                break;
            case 0x0E:
                if(!cf) pc++;
                break;
            case 0x0F:
                if(cf) pc++;
                break;
            }
            break;
        case 0x10:
            regs[memory[pc]&0x0f] = little(*(uint16_t*)&memory[pc+1]);
            break;
        case 0x20:
            a = regs[memory[pc]&0x0f];
            zf = !a;
            break;
        case 0x30:
            regs[memory[pc]&0x0f] = a;
            break;
        case 0x40:
            a = little(*(uint16_t*)&memory[regs[memory[pc]&0x0f]]);
            zf = !a;
            break;
        case 0x50:
            *(uint16_t*)&memory[regs[memory[pc]&0x0f]] = little(a);
            break;
        case 0x60:
            a = little(memory[regs[memory[pc]&0x0f]]);
            zf = !a;
            break;
        case 0x70:
            memory[regs[memory[pc]&0x0f]] = a;
            break;
        case 0x80:
            a += regs[memory[pc&0x0f]];
            zf = !a;
            cf = a < regs[memory[pc&0x0f]];
            break;
        case 0x90:
            cf = a <= regs[memory[pc&0x0f]];
            a -= regs[memory[pc&0x0f]];
            zf = !a;
            break;
        case 0xA0:
            regs[memory[pc&0x0f]]++;
            zf = !regs[memory[pc&0x0f]];
            cf = zf;
            break;
        case 0xB0:
            cf = !regs[memory[pc&0x0f]];
            regs[memory[pc&0x0f]]--;
            zf = !regs[memory[pc&0x0f]];
            break;
        case 0xC0:
            cf = a <= regs[memory[pc&0x0f]];
            zf = a == regs[memory[pc&0x0f]];
            break;
        case 0xD0:
            a &= regs[memory[pc&0x0f]];
            zf = !a;
            break;
        case 0xE0:
            a |= regs[memory[pc&0x0f]];
            zf = !a;
            break;
        case 0xF0:
            a ^= regs[memory[pc&0x0f]];
            zf = !a;
            break;
        }
        pc += insSize(memory[pc]);
    }
}
