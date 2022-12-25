#include "scvm.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

unsigned char memory[MEMORY_SIZE] = {0};
unsigned char rsp = 0;
uint16_t rpc = 0;
uint16_t regs[16] = {0};
uint16_t acc = 0;
bool zf = 0, cf = 0;
bool debugEnabled = false;

uint16_t get16(uint16_t m) {
    return (uint16_t)memory[m] | ((uint16_t)memory[m+1] << 8);
}

void set16(uint16_t m, uint16_t s) {
    memory[m] = s;
    memory[m+1] = s >> 8;
}

int insSize(unsigned char ins) {
    if((ins & 0xf0) == 0x10) return 3;
    if((ins & 0xf0) != 0) return 1;
    if((ins & 0x08) == 0) return 2;
    if((ins & 0x0e) == 0x0e) return 3;
    return 1;
}

int run() {
    unsigned char ins;
    for(;;) {
        if(debugEnabled) {
            printf("zf = %d  cf = %d\n", zf, cf);
            printf("acc = %.4x\n", acc);
            printf("%.4x %.2x ", rpc, memory[rpc]);
            switch(insSize(memory[rpc])) {
            case 2: printf("%.2x", memory[rpc+1]); break;
            case 3: printf("%.4x", get16(rpc+1)); break;
            }
            printf("\n");
        }
        ins = memory[rpc];
        switch(ins & 0xf0) {
        case 0x00:
            switch(ins) {
            case 0x00:
                rpc += 2;
                return memory[rpc-1];
            case 0x01:
                rpc += (char)memory[rpc+1] + 2;
                continue;
            case 0x02:
                if(zf) { rpc += (char)memory[rpc+1] + 2; continue; }
                break;
            case 0x03:
                if(!zf) { rpc += (char)memory[rpc+1] + 2; continue; }
                break;
            case 0x04:
                if(cf) { rpc += (char)memory[rpc+1] + 2; continue; }
                break;
            case 0x05:
                if(!cf) { rpc += (char)memory[rpc+1] + 2; continue; }
                break;
            case 0x08:
                cf = acc & 0x8000;
                acc <<= 1;
                zf = !acc;
                break;
            case 0x09:
                cf = acc & 1;
                acc >>= 1;
                zf = !acc;
                break;
            case 0x0A:
                acc = ~acc;
                zf = !acc;
                break;
            case 0x0B:
                set16(rsp, acc);
                rsp += 2;
                break;
            case 0x0C:
                rsp -= 2;
                acc = get16(rsp);
                break;
            case 0x0D:
                rsp -= 2;
                rpc = get16(rsp);
                break;
            case 0x0E:
                set16(rsp, rpc);
                rsp += 2;
                rpc = get16(rpc+1);
                continue;
            case 0x0F:
                acc = get16(rpc+1);
                break;
            }
            break;
        case 0x10:
            regs[ins&0x0f] = get16(rpc+1);
            break;
        case 0x20:
            acc = regs[ins&0x0f];
            zf = !acc;
            break;
        case 0x30:
            regs[ins&0x0f] = acc;
            break;
        case 0x40:
            acc = get16(regs[ins&0x0f]);
            zf = !acc;
            break;
        case 0x50:
            set16(regs[ins&0x0f], acc);
            break;
        case 0x60:
            acc = memory[regs[ins&0x0f]];
            zf = !acc;
            break;
        case 0x70:
            memory[regs[ins&0x0f]] = acc;
            break;
        case 0x80:
            acc += regs[ins&0x0f];
            zf = !acc;
            cf = acc < regs[ins&0x0f];
            break;
        case 0x90:
            cf = acc <= regs[ins&0x0f];
            acc -= regs[ins&0x0f];
            zf = !acc;
            break;
        case 0xA0:
            cf = acc <= regs[ins&0x0f];
            zf = acc == regs[ins&0x0f];
            break;
        case 0xB0:
            regs[ins&0x0f]++;
            zf = !regs[ins&0x0f];
            cf = zf;
            break;
        case 0xC0:
            cf = !regs[ins&0x0f];
            regs[ins&0x0f]--;
            zf = !regs[ins&0x0f];
            break;
        case 0xD0:
            acc &= regs[ins&0x0f];
            zf = !acc;
            break;
        case 0xE0:
            acc |= regs[ins&0x0f];
            zf = !acc;
            break;
        case 0xF0:
            acc ^= regs[ins&0x0f];
            zf = !acc;
            break;
        }
        rpc += insSize(memory[rpc]);
    }
}
