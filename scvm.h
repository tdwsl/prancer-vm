#ifndef SCVM_H
#define SCVM_H

#include <stdint.h>
#include <stdbool.h>

#define MEMORY_SIZE (4*1024*1024)

extern unsigned char memory[MEMORY_SIZE];
extern unsigned char rsp;
extern uint16_t rpc;
extern uint16_t regs[16];
extern uint16_t acc;
extern uint16_t bank;
extern bool zf, cf;
extern bool debugEnabled;

uint16_t getMemory(uint16_t m);
void setMemory(uint16_t m, uint16_t b);

int insSize(unsigned char ins);
int run();

#endif
