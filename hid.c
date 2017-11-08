#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

int calcPar2(uint32_t hi, uint32_t lo) {
  int p = 0;
  for(int i=1; i < 32; i+=3) {
    p ^= ((lo >> i) & 1) ^ ((lo >> (i+1)) & 1);
  }
  p ^= ((lo >> 31) & 1) ^ (hi & 1); // Even Parity
  printf("2p : %d\n", p);
  return p;
}

int calcPar35(uint32_t hi, uint32_t lo) {
  int p = 0;
  for(int i=2; i < 32; i+=3) {
    p ^= ((lo >> i) & 1) ^ ((lo >> (i+1)) & 1);
  }
  p ^= ((hi >> 1) & 1) ^ (hi & 1) ^ 1; // Odd Parity
  printf("35p: %d\n", p);
  return p;
}

int calcPar1(uint32_t hi, uint32_t lo) {
  int p = 0;
  for(int i=0;i<32;i++) {
    p ^= (lo >> i) & 1;
  }
  for(int i=0;i<2;i++) {
    p ^= (hi >> i) & 1;
  }
  p ^= 1; // Odd Parity
  printf("1p : %d\n", p);
  return p;
}

int main(int argc, char** argv) {
  if (argc != 3) {
    printf("Usage: hid35 fc cardnum\n");
    return -1;
  }

  int fc = atoi(argv[1]);
  int crd = atoi(argv[2]);
  uint32_t hi=0, lo=0;

  printf("HID 26 bit   Card Number %d\n", crd & 0xFFFF);
  printf("HID 35 bit FC %d CardNum %d\n", fc, crd);
  hi = (fc >> 11) & 0x0001;
  lo = ((fc & 0x7FF) << 21) | (crd & 0xFFFFF) << 1; 
  hi |= (calcPar2(hi, lo) << 1) & 0x02;
  lo |= calcPar35(hi, lo) & 1;
  hi |= (calcPar1(hi, lo) << 2) & 0x04;
  hi |= 0x0028;
  printf("HI : %02X ", hi);
  printf("LO : %08X\n", lo);
  printf("Hex: %02X%08X\n", hi, lo);

  uint64_t full = lo | ((uint64_t)hi << 32);
  printf("64b: %0lX\n", full);
 
  // T5577 Programming
  // Block 1
  // 00011101
  // 01010101
  // 01011001
  // 10...... depending on Data
  // Block 2
  // Block 3
  
  uint8_t bytes[12];
  int b = 0;  // point to current byte to fill

  bytes[b++] = 0x1D;
  bytes[b++] = 0x55;
  bytes[b++] = 0x59;

  // Manchester Encode the Data
  int s = 35;  // Shift value for current bit
  for(int i = 0; i < 9; i++) {
    uint8_t v = 0;
    v |= ((full >> s--) & 1) ? 0x80 : 0x40;
    v |= ((full >> s--) & 1) ? 0x20 : 0x10;
    v |= ((full >> s--) & 1) ? 0x08 : 0x04;
    v |= ((full >> s--) & 1) ? 0x02 : 0x01;
    bytes[b++] = v;
  }

  printf("T5577: 0x00107060-");
  for(int i=0; i < sizeof(bytes); i++) {
    printf("%02X", bytes[i]);
    if ((i==3) || (i ==7)) printf("-");
  }
  printf("\n");

  return 0;
}

