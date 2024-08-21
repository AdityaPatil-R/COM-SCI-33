/*
 * CS:APP Data Lab
 *
 * <Aditya Patil 406216539>
 *
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting if the shift amount
     is less than 0 or greater than 31.


EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }


NOTES:
  1. Our checker requires that you do NOT define a variable after 
     a statement that does not define a variable.

     For example, this is NOT allowed:

     int illegal_function_for_this_lab(int x, int y) {
       // this statement doesn't define a variable
       x = x + y + 1;
       
       // The checker for this lab does NOT allow the following statement,
       // because this variable definition comes after a statement 
       // that doesn't define a variable
       int z;

       return 0;
     }
  2. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  3. Each function has a maximum number of operations (! ~ & ^ | + << >>)
     that you are allowed to use for your implementation of the function.
     The max operator count is checked by dlc. Note that assignment ('=')
     is not counted; you may use as many of these as you want without penalty.
  4. Use the btest test harness to check your functions for correctness.
  5. The maximum number of ops for each function is given in the
     header comment for each function.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use btest to formally verify that your solutions produce 
 *      the correct answers.
 */

#endif

/*
 * tmin - return minimum two's complement integer
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmin(void)
{
    /*
     * 0000 0000 0000 0000 0000 0000 0000 0001 -> 1000 0000 0000 0000 0000 0000 0000 0000
     * This is -2^31, the lowest possible two's complement integer
     */

    return 1 << 31;
}

/*
 * isTmax - returns 1 if x is the maximum, two's complement number,
 *          and 0 otherwise
 *   Legal ops: ! ~ & ^ | +
 *   Max ops: 10
 *   Rating: 1
 */
int isTmax(int x)
{
    /*
     * If x is Tmax, y will be Tmin, and negating x + y will result in 0
     * !y will handle the case where x is -1
     */

    int y = x + 1;
    x = ~(x + y);
    y = !y;

    return !(x + y);
}

/*
 * bitXor - x^y using only ~ and &
 *   Example: bitXor(4, 5) = 1
 *   Legal ops: ~ &
 *   Max ops: 14
 *   Rating: 1
 */
int bitXor(int x, int y)
{
    /*
     * De Morgan's Law is used
     * x ^ y => (x & ~y) | (~x & y) => ~(~(x & ~y) & ~(~x & y)
     */

    return ~(~(x & ~y) & ~(~x & y));
}

/*
 * sign - return 1 if positive, 0 if zero, and -1 if negative
 *   Examples: sign(130) = 1
 *             sign(-23) = -1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 10
 *   Rating: 2
 */
int sign(int x)
{
    /*
     * Checks if x is negative or if it's zero
     * If x is negative, 1111 1111 1111 1111 1111 1111 1111 1111 is returned
     * If not, return value depends on if x is 0
     */

    int isNegative = x >> 31;
    int isZero = !x;

    return isNegative | !isZero;
}

/*
 * fitsBits - return 1 if x can be represented as an
 *            n-bit, two's complement integer.
 *   1 <= n <= 32
 *   Examples: fitsBits(5, 3) = 0
 *             fitsBits(-4, 3) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 2
 */
int fitsBits(int x, int n)
{
    /*
     * Moves x back and forth by 32 - n bits
     * Checks if x is the same as the original value
     */

    int move = 32 + (~n + 1);
    int backAndForth = (x << move) >> move;

    return !(x ^ backAndForth);
}

/*
 * byteSwap - swaps the nth byte and the mth byte
 *   Examples: byteSwap(0x12345678, 1, 3) = 0x56341278
 *             byteSwap(0xDEADBEEF, 0, 2) = 0xDEEFBEAD
 *   You may assume that 0 <= n <= 3, 0 <= m <= 3
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 25
 *   Rating: 2
 */
int byteSwap(int x, int n, int m)
{
    /*
     * Shifts nth and mth bytes to the right and isolates them
     * Masks them and unmask the original value
     * Swaps the nth and mth bytes and combines it with the original
     */

    int nShift = n << 3;
    int mShift = m << 3;

    int nthByte = (x >> nShift) & 255;
    int mthByte = (x >> mShift) & 255;

    int nMask = 255 << nShift;
    int mMask = 255 << mShift;

    int unmask = ~(nMask | mMask);
    x = x & unmask;

    int mToN = mthByte << nShift;
    int nToM = nthByte << mShift;

    return x | mToN | nToM;
}

/*
 * isAsciiDigit - return 1 if 0x30 <= x <= 0x39
 *                (ASCII codes for characters '0' to '9')
 *   Examples: isAsciiDigit(0x35) = 1
 *             isAsciiDigit(0x3a) = 0
 *             isAsciiDigit(0x05) = 0
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 3
 */
int isAsciiDigit(int x)
{
    /*
     * Computes distance of x from lower and upper bounds
     * Checks if distances are positive to ensure x is between bounds
     */

    int lower = 0x30;
    int upper = 0x39;

    int xMinusLower = x + ~lower + 1;
    int upperMinusX = upper + ~x + 1;

    int notBelowLower = !(xMinusLower >> 31);
    int notAboveUpper = !(upperMinusX >> 31);

    return !!(notBelowLower & notAboveUpper);
}

/*
 * conditional - same as x ? y : z
 *   Example: conditional(2, 4, 5) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
int conditional(int x, int y, int z)
{
    /*
     * See if x is true or false and create a mask
     * Depending on the mask, return y or z
     */

    int trueOrFalse = !!x;
    int mask = ~trueOrFalse + 1;

    return (mask & y) | (~mask & z);
}

/*
 * subtractionOK - Determine if can compute x-y without overflow
 *   Examples: subtractionOK(0x80000000, 0x80000000) = 1
 *             subtractionOK(0x80000000, 0x70000000) = 0
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 3
 */
int subtractionOK(int x, int y)
{
    /*
     * Checks for overflow with signs of x, y, and x - y
     * If x and y have the same sign, x - y should have the same sign
     */

    int result = x + (~y + 1);

    int xSign = (x >> 31) & 1;
    int ySign = (y >> 31) & 1;

    int resultSign = (result >> 31) & 1;

    int overflow = (xSign ^ ySign) & (xSign ^ resultSign);

    return !overflow;
}

/*
 * rotateRight - Rotate x to the right by n
 *   Can assume that 0 <= n <= 31
 *   Example: rotateRight(0x87654321, 4) = 0x18765432
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 25
 *   Rating: 3
 */
int rotateRight(int x, int n)
{
    /*
     * Moves leftmost n bits to the rightmost n bits
     * Moves rightmost 32 - n bits to the leftmost 32 - n bits
     * Combines the two
     */

    int rightMask = (1 << n) + ~0;
    int rightBits = x & rightMask;
    int rightToLeft = rightBits << (32 + ~n + 1);

    int leftMask = ~(((1 << 31) >> n) << 1);
    int leftToRight = x >> n;
    int leftBits = leftToRight & leftMask;

    return rightToLeft | leftBits;
}

// Below are the extra credit problems (4 pts in total)
// 2 points each (1 correctness pt + 1 performance pt)

/*
 * bitParity - returns 1 if x contains an odd number of 0's
 *   Examples: bitParity(5) = 0
 *             bitParity(7) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 */
int bitParity(int x)
{
    /*
     * Iteratively reduce the problem to checking the parity of lesser significant bits
     * Once reduced to the rightmost bit, return its parity
     */

    x = x ^ (x >> 16);
    x = x ^ (x >> 8);
    x = x ^ (x >> 4);
    x = x ^ (x >> 2);
    x = x ^ (x >> 1);

    return x & 1;
}

/*
 * greatestBitPos - return a mask that marks the position of the
 *                  most significant 1 bit. If x == 0, return 0
 *   Example: greatestBitPos(96) = 0x40
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 70
 */
int greatestBitPos(int x)
{
    /*
     * Store the sign and make all bits to the right of the leftmost bit 1
     * Isolate the leftmost bit and return it, taking note of the sign
     */

    int sign = x & (1 << 31);

    x = x | (x >> 1);
    x = x | (x >> 2);
    x = x | (x >> 4);
    x = x | (x >> 8);
    x = x | (x >> 16);

    int leftmostBit = x & ~(x >> 1);

    return leftmostBit | sign;
}