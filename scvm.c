#include "scvm.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

unsigned char memory[MEMORY_SIZE] = {0};
unsigned char rsp = 0;
uint32_t rpc = 0;
uint32_t regs[16] = {0};
uint32_t acc = 0;
bool zf = 0, cf = 0;
bool debugEnabled = false;

uint32_t get24(uint32_t m) {
    return (uint32_t)memory[m] | ((uint32_t)memory[m+1] << 8)
            | ((uint32_t)memory[m+2] << 16);
}

void set24(uint32_t m, uint32_t s) {
    memory[m] = s;
    memory[m+1] = s >> 8;
    memory[m+2] = s >> 16;
}

int insSize(unsigned char ins) {
    if(ins == 0) return 2;
    if((ins & 0xf0) == 0x10) return 4;
    if((ins & 0xf0) != 0) return 1;
    if((ins & 0x08) == 0) return 1;
    if((ins & 0x04) == 0) return 4;
    return 1;
}

int run() {
    unsigned char ins;
    for(;;) {
        if(debugEnabled) {
            printf("zf = %d  cf = %d\n", zf, cf);
            printf("acc = %.6x\n", acc);
            printf("%.6x %.2x\n", rpc, memory[rpc]);
        }
        ins = memory[rpc];
        switch(ins & 0xf0) {
        case 0x00:
            switch(ins) {
            case 0x00:
                rpc += 2;
                rpc &= 0xffffff;
                return memory[rpc-1];
            case 0x01:
                cf = acc & 0x800000;
                acc <<= 1;
                acc &= 0xffffff;
                zf = !acc;
                break;
            case 0x02:
                cf = acc & 1;
                acc >>= 1;
                zf = !acc;
                break;
            case 0x03:
                acc = ~acc;
                acc &= 0xffffff;
                zf = !acc;
                break;
            case 0x04:
                set24(rsp, acc);
                rsp += 3;
                break;
            case 0x05:
                rsp -= 3;
                acc = get24(rsp);
                break;
            case 0x06:
                rsp -= 3;
                rpc = get24(rsp);
                break;
            case 0x07:
                if(acc & 0x80) acc |= 0xffff00;
                else acc &= 0x0000ff;
                zf = !acc;
                break;
            case 0x08:
                set24(rsp, rpc);
                rsp += 3;
                rpc = get24(rpc+1);
                continue;
            case 0x09:
                rpc = get24(rpc+1);
                continue;
            case 0x0A:
                acc = get24(rpc+1);
                break;
            case 0x0C:
                if(!zf) { rpc++; rpc &= 0xffffff; }
                break;
            case 0x0D:
                if(zf) { rpc++; rpc &= 0xffffff; }
                break;
            case 0x0E:
                if(!cf) { rpc++; rpc &= 0xffffff; }
                break;
            case 0x0F:
                if(cf) { rpc++; rpc &= 0xffffff; }
                break;
            }
            break;
        case 0x10:
            regs[ins&0x0f] = get24(rpc+1);
            break;
        case 0x20:
            acc = regs[ins&0x0f];
            zf = !acc;
            break;
        case 0x30:
            regs[ins&0x0f] = acc;
            break;
        case 0x40:
            acc = get24(regs[ins&0x0f]);
            zf = !acc;
            break;
        case 0x50:
            set24(regs[ins&0x0f], acc);
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
            acc &= 0xffffff;
            zf = !acc;
            cf = acc < regs[ins&0x0f];
            break;
        case 0x90:
            cf = acc <= regs[ins&0x0f];
            acc -= regs[ins&0x0f];
            acc &= 0xffffff;
            zf = !acc;
            break;
        case 0xA0:
            cf = acc <= regs[ins&0x0f];
            zf = acc == regs[ins&0x0f];
            break;
        case 0xB0:
            regs[ins&0x0f]++;
            regs[ins&0x0f] &= 0xffffff;
            zf = !regs[ins&0x0f];
            cf = zf;
            break;
        case 0xC0:
            cf = !regs[ins&0x0f];
            regs[ins&0x0f]--;
            regs[ins&0x0f] &= 0xffffff;
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
        rpc &= 0xffffff;
    }
}
