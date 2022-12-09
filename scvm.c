#include "scvm.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

unsigned char memory[MEMORY_SIZE] = {0};
unsigned char rsp = 0;
uint16_t rpc = 0;
uint16_t regs[16] = {0};
uint16_t acc = 0;
uint16_t bank = 0;
bool zf = 0, cf = 0;
bool debugEnabled = false;

uint16_t getMemory(uint16_t m) {
    if(m < 32768) return memory[m];
    return memory[((uint32_t)bank << 7) | m];
}

void setMemory(uint16_t m, uint16_t b) {
    if(m < 32768) memory[m] = b;
    else memory[((uint32_t)bank << 7) | m] = b;
}

int insSize(unsigned char ins) {
    if(ins == 0) return 2;
    if((ins & 0xf0) == 0x10) return 3;
    if((ins & 0xf0) != 0) return 1;
    if((ins & 0x08) == 0) return 1;
    if((ins & 0x04) == 0) return 3;
    return 1;
}

int run() {
    unsigned char ins;
    for(;;) {
        if(debugEnabled) {
            printf("zf = %d  cf = %d\n", zf, cf);
            printf("%.4x %.2x\n", rpc, getMemory(rpc));
        }
        ins = getMemory(rpc);
        switch(ins & 0xf0) {
        case 0x00:
            switch(ins) {
            case 0x00:
                rpc += 2;
                return getMemory(rpc-1);
            case 0x01:
                cf = acc & 0x8000;
                acc <<= 1;
                zf = !acc;
                break;
            case 0x02:
                cf = acc & 1;
                acc >>= 1;
                zf = !acc;
                break;
            case 0x03:
                acc = ~acc;
                zf = !acc;
                break;
            case 0x04:
                setMemory(rsp, acc);
                setMemory(rsp, acc >> 8);
                rsp += 2;
                break;
            case 0x05:
                rsp -= 2;
                acc = getMemory(rsp) | (getMemory(rsp+1) << 8);
                break;
            case 0x06:
                rsp -= 2;
                rpc = getMemory(rsp) | (getMemory(rsp+1) << 8);
                break;
            case 0x07:
                if(acc & 0x80) acc |= 0xff00;
                else acc &= 0x00ff;
                zf = !acc;
                break;
            case 0x08:
                setMemory(rsp, rpc);
                setMemory(rsp, rpc << 8);
                rsp += 2;
                rpc = getMemory(rpc+1) | (getMemory(rpc+2) << 8);
                continue;
            case 0x09:
                rpc = getMemory(rpc+1) | (getMemory(rpc+2) << 8);
                continue;
            case 0x0A:
                acc = getMemory(rpc+1) | (getMemory(rpc+2) << 8);
                break;
            case 0x0C:
                if(!zf) rpc++;
                break;
            case 0x0D:
                if(zf) rpc++;
                break;
            case 0x0E:
                if(!cf) rpc++;
                break;
            case 0x0F:
                if(cf) rpc++;
                break;
            }
            break;
        case 0x10:
            regs[ins&0x0f] = getMemory(rpc+1) | (getMemory(rpc+2) << 8);
            break;
        case 0x20:
            acc = regs[ins&0x0f];
            zf = !acc;
            break;
        case 0x30:
            regs[ins&0x0f] = acc;
            break;
        case 0x40:
            acc = getMemory(regs[ins&0x0f])
                    | (getMemory(regs[ins&0x0f]+1) << 8);
            zf = !acc;
            break;
        case 0x50:
            setMemory(regs[ins&0x0f], acc);
            setMemory(regs[ins&0x0f]+1, acc >> 8);
            break;
        case 0x60:
            acc = getMemory(regs[ins&0x0f]);
            zf = !acc;
            break;
        case 0x70:
            setMemory(regs[ins&0x0f], acc);
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
        rpc += insSize(getMemory(rpc));
    }
}
