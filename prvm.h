#ifndef PRVM_H
#define PRVM_H

#include <stdint.h>
#include <stdbool.h>

#define MEMORY_SIZE 65536

extern unsigned char memory[MEMORY_SIZE];
extern unsigned char rsp;
extern uint16_t rpc;
extern uint16_t regs[16];
extern uint16_t acc;
extern bool zf, cf;
extern bool debugEnabled;

uint16_t get16(uint16_t m);
void set16(uint16_t m, uint16_t b);

int insSize(unsigned char ins);
int run();

#endif
