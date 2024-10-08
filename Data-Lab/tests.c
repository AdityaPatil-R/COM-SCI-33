/* Testing Code */

#include <limits.h>
#include <math.h>

/* Routines used by floation point test code */

/* Convert from bit level representation to floating point number */
float u2f(unsigned u) {
  union {
    unsigned u;
    float f;
  } a;
  a.u = u;
  return a.f;
}

/* Convert from floating point number to bit-level representation */
unsigned f2u(float f) {
  union {
    unsigned u;
    float f;
  } a;
  a.f = f;
  return a.u;
}

int test_tmin(void) {
  return 0x80000000;
}

int test_isTmax(int x) {
  return x == 0x7FFFFFFF;
}

int test_bitXor(int x, int y) {
  return x^y;
}

int test_sign(int x) {
  if ( !x ) return 0;
  return (x < 0) ? -1 : 1;
}

int test_fitsBits(int x, int n) {
  int TMin_n = -(1 << (n-1));
  int TMax_n = (1 << (n-1)) - 1;
  return x >= TMin_n && x <= TMax_n;
}

int test_byteSwap(int x, int n, int m) {
  /* little endian machine */
  /* least significant byte stored first */
  unsigned int nmask, mmask;
  switch(n) {
    case 0:
      nmask = x & 0xFF;
      x &= 0xFFFFFF00;
      break;
    case 1:
      nmask = (x & 0xFF00) >> 8;
      x &= 0xFFFF00FF;
      break;
    case 2:
      nmask = (x & 0xFF0000) >> 16;
      x &= 0xFF00FFFF;
      break;
    default:
      nmask = ((unsigned int)(x & 0xFF000000)) >> 24;
      x &= 0x00FFFFFF;
      break;
  }
  switch(m) {
    case 0:
      mmask = x & 0xFF;
      x &= 0xFFFFFF00;
      break;
    case 1:
      mmask = (x & 0xFF00) >> 8;
      x &= 0xFFFF00FF;
      break;
    case 2:
      mmask = (x & 0xFF0000) >> 16;
      x &= 0xFF00FFFF;
      break;
    default:
      mmask = ((unsigned int)(x & 0xFF000000)) >> 24;
      x &= 0x00FFFFFF;
      break;
  }
  nmask <<= 8*m;
  mmask <<= 8*n;
  return x | nmask | mmask;
}

int test_isAsciiDigit(int x) {
  return (0x30 <= x) && (x <= 0x39);
}

int test_conditional(int x, int y, int z) {
  return x ? y : z;
}

int test_subtractionOK(int x, int y) {
  long long ldiff = (long long) x - y;
  return ldiff == (int) ldiff;
}

int test_rotateRight(int x, int n) {
  unsigned u = (unsigned) x;
  int i;
  for (i = 0; i < n; i++) {
    unsigned lsb = (u & 1) << 31;
    unsigned rest = u >> 1;
    u = lsb | rest;
  }
  return (int) u;
}

int test_bitParity(int x) {
  int result = 0;
  int i;
  for (i = 0; i < 32; i++)
    result ^= (x >> i) & 0x1;
  return result;
}

int test_greatestBitPos(int x) {
  unsigned mask = 1<<31;
  if (x == 0)
    return 0;
  while (!(mask & x)) {
    mask = mask >> 1;
  }
  return mask;
}

