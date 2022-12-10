#ifndef SCVM_H
#define SCVM_H

#include <stdint.h>
#include <stdbool.h>

#define MEMORY_SIZE (4*1024*1024)

extern unsigned char memory[MEMORY_SIZE];
extern unsigned char rsp;
extern uint32_t rpc;
extern uint32_t regs[16];
extern uint32_t acc;
extern bool zf, cf;
extern bool debugEnabled;

uint32_t get24(uint32_t m);
void set24(uint32_t m, uint32_t b);

int insSize(unsigned char ins);
int run();

#endif
