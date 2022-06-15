#ifndef _mcp3008_h
#define _mcp3008_h
#include <stdint.h>

//static int Prepare(int fd);
uint8_t ControlBitsDifferential(uint8_t cannel);
uint8_t ControlBits(uint8_t channel);
int ReadADC(int fd, uint8_t channel);
int * RunMCP();

#endif