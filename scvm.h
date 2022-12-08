#ifndef SCVM_H
#define SCVM_H

#include <stdint.h>
#include <stdbool.h>

#ifdef SC_BIG_ENDIAN
#include <byteswap.h>
#define little(A) __bswap_16(A)
#else
#define little(A) (A)
#endif

extern unsigned char memory[65536];
extern unsigned char sp;
extern uint16_t pc;
extern uint16_t regs[16];
extern uint16_t a;
extern bool zf, cf;

int insSize(unsigned char ins);
int run();

#endif
