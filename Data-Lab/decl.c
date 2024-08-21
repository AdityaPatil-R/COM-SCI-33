#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#define TMin INT_MIN
#define TMax INT_MAX

#include "btest.h"
#include "bits.h"

test_rec test_set[] = {
 {"tmin", (funct_t) tmin, (funct_t) test_tmin, 0,
  "! ~ & ^ | + << >>", 4, 1,
  {{TMin, TMax},{TMin,TMax},{TMin,TMax}}},

 {"isTmax", (funct_t) isTmax, (funct_t) test_isTmax, 1,
  "! ~ & ^ | +", 10, 1,
  {{TMin, TMax},{TMin,TMax},{TMin,TMax}}},

 {"bitXor", (funct_t) bitXor, (funct_t) test_bitXor, 2,
  "& ~", 14, 1,
  {{TMin, TMax},{TMin,TMax},{TMin,TMax}}},

 {"sign", (funct_t) sign, (funct_t) test_sign, 1,
  "! ~ & ^ | + << >>", 10, 2,
  {{-TMax, TMax},{TMin,TMax},{TMin,TMax}}},

 {"fitsBits", (funct_t) fitsBits, (funct_t) test_fitsBits, 2,
  "! ~ & ^ | + << >>", 15, 2,
  {{TMin, TMax},{1,32},{TMin,TMax}}},

 {"byteSwap", (funct_t) byteSwap, (funct_t) test_byteSwap, 3,
  "! ~ & ^ | + << >>", 25, 2,
  {{TMin, TMax},{0,3},{0,3}}},

 {"isAsciiDigit", (funct_t) isAsciiDigit, (funct_t) test_isAsciiDigit, 1,
  "! ~ & ^ | + << >>", 15, 3,
  {{TMin, TMax},{TMin,TMax},{TMin,TMax}}},

 {"conditional", (funct_t) conditional, (funct_t) test_conditional, 3,
  "! ~ & ^ | << >>", 16, 3,
  {{TMin, TMax},{TMin,TMax},{TMin,TMax}}},

 {"subtractionOK", (funct_t) subtractionOK, (funct_t) test_subtractionOK, 2,
  "! ~ & ^ | + << >>", 20, 3,
  {{TMin, TMax},{TMin,TMax},{TMin,TMax}}},

 {"rotateRight", (funct_t) rotateRight, (funct_t) test_rotateRight, 2,
  "! ~ & ^ | + << >>", 25, 3,
  {{TMin, TMax},{0,31},{TMin,TMax}}},

 {"bitParity", (funct_t) bitParity, (funct_t) test_bitParity, 1,
  "! ~ & ^ | + << >>", 20, 1,
  {{TMin, TMax},{TMin,TMax},{TMin,TMax}}},

 {"greatestBitPos", (funct_t) greatestBitPos, (funct_t) test_greatestBitPos, 1,
  "! ~ & ^ | + << >>", 70, 1,
  {{TMin, TMax},{TMin,TMax},{TMin,TMax}}},

  {"", NULL, NULL, 0, "", 0, 0,
   {{0, 0},{0,0},{0,0}}}
};
