/* Test suite for the library.  First, it ``tests'' that all the constructs it
 * uses compile successfully.  Then, its output to stdout is compared to the
 * expected output automatically extracted from slash-slash comments below.
 */

#include "BigIntegerLibrary.hh"

#include <string>
#include <iostream>

unsigned long MAX_UNSIGNED = 0xffffffffffffffffU;
long MAX_SIGNED = 0x7fffffffffffffffL;
long MIN_SIGNED = 0x8000000000000000U;
char MIN_UNSIGNED_OVERFLOW[] = "18446744073709551616";
char MIN_SIGNED_OVERFLOW[] = "9223372036854775807";
char MAX_SIGNED_UNDERFLOW[] = "-9223372036854775809";

using namespace std;

// Evaluate expr and print the result or "error" as appropriate.
#define TEST(expr) do {\
	cout << "Line " << __LINE__ << ": ";\
	try {\
		cout << (expr);\
	} catch (const char *err) {\
		cout << "error";\
	}\
	cout << endl;\
} while (0)

const BigUnsigned &check(const BigUnsigned &x) {
	unsigned int l = x.getLength();
	if (l != 0 && x.getBlock(l-1) == 0)
		cout << "check: Unzapped number!" << endl;
	if (l > x.getCapacity())
		cout << "check: Capacity inconsistent with length!" << endl;
	return x;
}

const BigInteger &check(const BigInteger &x) {
	if (x.getSign() == 0 && !x.getMagnitude().isZero())
		cout << "check: Sign should not be zero!" << endl;
	if (x.getSign() != 0 && x.getMagnitude().isZero())
		cout << "check: Sign should be zero!" << endl;
	check(x.getMagnitude());
	return x;
}

short pathologicalShort = ~((unsigned short)(~0) >> 1);
int pathologicalInt = ~((unsigned int)(~0) >> 1);
long pathologicalLong = ~((unsigned long)(~0) >> 1);

int main() {

try {

BigUnsigned z(0), one(1), ten(10);
TEST(z); //0
TEST(1); //1
TEST(10); //10

// TODO: Comprehensively test the general and special cases of each function.

// === Default constructors ===

TEST(check(BigUnsigned())); //0
TEST(check(BigInteger())); //0

// === Block-array constructors ===


BigUnsigned::Blk myBlocks[4];
myBlocks[0] = 0x0000000400000003;
myBlocks[1] = 0x0000000200000005;
myBlocks[2] = 0;
myBlocks[3] = 1; /* A "trap" to make sure the constructor doesn't read too much data. */
int num_blocks = 3;

BigUnsigned bu(myBlocks, num_blocks);
TEST(check(bu)); //158456325120762395572815527939
TEST(check(BigInteger(myBlocks, num_blocks))); //158456325120762395572815527939
TEST(check(BigInteger(bu         ))); //158456325120762395572815527939

// For nonzero magnitude, reject zero and invalid signs.
TEST(check(BigInteger(myBlocks, num_blocks, BigInteger::positive))); //158456325120762395572815527939
TEST(check(BigInteger(myBlocks, num_blocks, BigInteger::negative))); //-158456325120762395572815527939
TEST(check(BigInteger(myBlocks, num_blocks, BigInteger::zero    ))); //error
TEST(check(BigInteger(bu,          BigInteger::positive))); //158456325120762395572815527939
TEST(check(BigInteger(bu,          BigInteger::negative))); //-158456325120762395572815527939
TEST(check(BigInteger(bu,          BigInteger::zero    ))); //error

// For zero magnitude, force the sign to zero without error.
BigUnsigned::Blk myZeroBlocks[1];
myZeroBlocks[0] = 0;
TEST(check(BigInteger(myZeroBlocks, 1, BigInteger::positive))); //0
TEST(check(BigInteger(myZeroBlocks, 1, BigInteger::negative))); //0
TEST(check(BigInteger(myZeroBlocks, 1, BigInteger::zero    ))); //0

// === BigUnsigned conversion limits ===

 TEST(BigUnsigned(0).toUnsignedLong()); //0
 TEST(BigUnsigned(MAX_UNSIGNED).toUnsignedLong()); //18446744073709551615
 TEST(stringToBigUnsigned(MIN_UNSIGNED_OVERFLOW).toUnsignedLong()); //error 

TEST(BigUnsigned(0).toLong()); //0
TEST(BigUnsigned(MAX_SIGNED).toLong()); //9223372036854775807
 TEST(BigUnsigned(((unsigned long)MAX_SIGNED) + 1UL).toLong()); //error

// int is the same as long on a 32-bit system
TEST(BigUnsigned(0).toUnsignedInt()); //0
TEST(BigUnsigned(4294967295U).toUnsignedInt()); //4294967295
TEST(stringToBigUnsigned("4294967296").toUnsignedInt()); //error

TEST(BigUnsigned(0).toInt()); //0
TEST(BigUnsigned(2147483647).toInt()); //2147483647
TEST(BigUnsigned(2147483648U).toInt()); //error

TEST(BigUnsigned(0).toUnsignedShort()); //0
TEST(BigUnsigned(65535).toUnsignedShort()); //65535
TEST(BigUnsigned(65536).toUnsignedShort()); //error

TEST(BigUnsigned(0).toShort()); //0
TEST(BigUnsigned(32767).toShort()); //32767
TEST(BigUnsigned(32768).toShort()); //error

// === BigInteger conversion limits ===

TEST(BigInteger(-1).toUnsignedLong()); //error
TEST(BigInteger(0).toUnsignedLong()); //0
TEST(BigInteger(MAX_UNSIGNED).toUnsignedLong()); //18446744073709551615
TEST(stringToBigInteger(MIN_UNSIGNED_OVERFLOW).toUnsignedLong()); //error

TEST(stringToBigInteger(MAX_SIGNED_UNDERFLOW).toLong()); //error
TEST(stringToBigInteger("-9223372036854775808").toLong()); //-9223372036854775808
TEST(BigInteger(-MAX_SIGNED -1).toLong()); //-9223372036854775808
TEST(BigInteger(0).toLong()); //0
TEST(BigInteger(MAX_SIGNED).toLong()); //9223372036854775807
TEST(BigInteger(((unsigned long)MAX_SIGNED) + 1UL).toLong()); //error

// Test to int
TEST(BigInteger(-1).toUnsignedInt()); //error
TEST(BigInteger(0).toUnsignedInt()); //0
TEST(BigInteger(4294967295U).toUnsignedInt()); //4294967295
TEST(stringToBigInteger("4294967296").toUnsignedInt()); //error

TEST(stringToBigInteger("-2147483649").toInt()); //error
TEST(stringToBigInteger("-2147483648").toInt()); //-2147483648
TEST(BigInteger(-2147483647).toInt()); //-2147483647
TEST(BigInteger(0).toInt()); //0
TEST(BigInteger(2147483647).toInt()); //2147483647
TEST(BigInteger(2147483648U).toInt()); //error

TEST(BigInteger(-1).toUnsignedShort()); //error
TEST(BigInteger(0).toUnsignedShort()); //0
TEST(BigInteger(65535).toUnsignedShort()); //65535
TEST(BigInteger(65536).toUnsignedShort()); //error

TEST(BigInteger(-32769).toShort()); //error
TEST(BigInteger(-32768).toShort()); //-32768
TEST(BigInteger(-32767).toShort()); //-32767
TEST(BigInteger(0).toShort()); //0
TEST(BigInteger(32767).toShort()); //32767
TEST(BigInteger(32768).toShort()); //error

// === Negative BigUnsigneds ===

// ...during construction
TEST(BigUnsigned(short(-1))); //error
TEST(BigUnsigned(pathologicalShort)); //error
TEST(BigUnsigned(-1)); //error
TEST(BigUnsigned(pathologicalInt)); //error
TEST(BigUnsigned(long(-1))); //error
TEST(BigUnsigned(pathologicalLong)); //error

// ...during subtraction
TEST(BigUnsigned(5) - BigUnsigned(6)); //error
TEST(stringToBigUnsigned("314159265358979323") - stringToBigUnsigned("314159265358979324")); //error
TEST(stringToBigUnsigned("323418446744073709551616") - stringToBigUnsigned("323418446744073709551617")); //error
TEST(check(BigUnsigned(5) - BigUnsigned(5))); //0
TEST(check(stringToBigUnsigned("314159265358979323") - stringToBigUnsigned("314159265358979323"))); //0
TEST(stringToBigUnsigned("323418446744073709551616") - stringToBigUnsigned("323418446744073709551616")); //0
TEST(stringToBigUnsigned("323418446744073709551616") - stringToBigUnsigned("323418446744073709551617")); //error
TEST(check(stringToBigUnsigned("4294967296") - BigUnsigned(1))); //4294967295
TEST(check(stringToBigUnsigned(MIN_UNSIGNED_OVERFLOW) - BigUnsigned(1))); //18446744073709551615

// === BigUnsigned addition ===

TEST(check(BigUnsigned(0) + 0)); //0
TEST(check(BigUnsigned(0) + 1)); //1
// Ordinary carry
TEST(check(stringToBigUnsigned("8589934591" /* 2^33 - 1*/)
		+ stringToBigUnsigned("4294967298" /* 2^32 + 2 */))); //12884901889
TEST(check(stringToBigUnsigned("295147905179352825856" /* 2^65 - 1*/)
		+ stringToBigUnsigned("4294967298" /* 2^32 + 2 */))); //295147905183647793154

// Creation of a new block
TEST(check(BigUnsigned(MAX_UNSIGNED) + 1)); //18446744073709551616

// === BigUnsigned subtraction ===

TEST(check(BigUnsigned(1) - 0)); //1
TEST(check(BigUnsigned(1) - 1)); //0
TEST(check(BigUnsigned(2) - 1)); //1
// Ordinary borrow
TEST(check(stringToBigUnsigned("295147905183647793154")
		- stringToBigUnsigned("4294967298"))); //295147905179352825856
// Borrow that removes a block
TEST(check(stringToBigUnsigned(MIN_UNSIGNED_OVERFLOW) - 1)); //18446744073709551615
TEST(check(stringToBigUnsigned(MIN_UNSIGNED_OVERFLOW) - 2)); //18446744073709551614

// === BigUnsigned multiplication and division ===

BigUnsigned a = check(BigUnsigned(314159265) * 358979323);
TEST(a); //112776680263877595
TEST(a / 123); //916883579381118
TEST(a % 123); //81



a = check(BigUnsigned(0x2312345678) * 0x4587654321);
TEST(a); //44981562471037640674680
TEST(a / 123); //365703759927135290038
TEST(a % 123); //6

 a = check(BigUnsignedInABase("ab1234567890abcdef", 16));
 BigUnsigned b1 = check(BigUnsignedInABase("b1234567890abcdef", 16));
TEST(a / b1); //15
TEST(a % b1); //92315705900128713966

BigUnsigned q;
 BigUnsigned c = check(BigUnsignedInABase("ab1234567890abcdef", 16));
 BigUnsigned d = check(BigUnsignedInABase("b1234567890abcdef", 16));
c.divideWithRemainder(d, q);
TEST(q); //15
TEST(c); //92315705900128713966

 c.divideWithRemainder(c, q);
 TEST(c); //0 
 TEST(q); //1

BigUnsigned e = check(BigUnsignedInABase("ab1234567890abcdef", 16));
 BigUnsigned f = check(BigUnsignedInABase("b1234567890abcdef", 16));
 e.divideWithRemainder(f, f);
 TEST(e); //92315705900128713966
 TEST(f); //15

TEST(BigUnsigned(5) / 0); //error

// === Block accessors ===

BigUnsigned b;
TEST(b); //0
TEST(b.getBlock(0)); //0
b.setBlock(1, 314);
// Did b grow properly?  And did we zero intermediate blocks?
TEST(check(b)); //5792277639144799207424
TEST(b.getLength()); //2
TEST(b.getBlock(0)); //0
TEST(b.getBlock(1)); //314
// Did b shrink properly?
b.setBlock(1, 0);
TEST(check(b)); //0

BigUnsigned bb(314);
bb.setBlock(1, 159);
// Make sure we used allocateAndCopy, not allocate
TEST(bb.getBlock(0)); //314
TEST(bb.getBlock(1)); //159
// Blocks beyond the number should be zero regardless of whether they are
// within the capacity.
bb.add(1, 2);
TEST(bb.getBlock(0)); //3
TEST(bb.getBlock(1)); //0
TEST(bb.getBlock(2)); //0
TEST(bb.getBlock(314159)); //0

// === Bit accessors ===

TEST(BigUnsigned(0).bitLength()); //0
TEST(BigUnsigned(1).bitLength()); //1
TEST(BigUnsigned(4095).bitLength()); //12
TEST(BigUnsigned(4096).bitLength()); //13
// 5 billion is between 2^32 (about 4 billion) and 2^33 (about 8 billion).
TEST(stringToBigUnsigned("5000000000").bitLength()); //33
TEST(stringToBigUnsigned("5634002666955639777930794616").bitLength()); //93

// 25 is binary 11001.
BigUnsigned bbb(25);
TEST(bbb.getBit(4)); //1
TEST(bbb.getBit(3)); //1
TEST(bbb.getBit(2)); //0
TEST(bbb.getBit(1)); //0
TEST(bbb.getBit(0)); //1
TEST(bbb.bitLength()); //5
// Effectively add 2^32.
bbb.setBit(32, true);
TEST(bbb); //4294967321
bbb.setBit(31, true);
bbb.setBit(32, false);
TEST(check(bbb)); //2147483673

BigUnsigned bb2(25);
 bb2.setBit(66, true);
 TEST(bb2); //73786976294838206489

// === Combining BigUnsigned, BigInteger, and primitive integers ===

BigUnsigned p1 = BigUnsigned(3) * 5;
TEST(p1); //15
/* In this case, we would like g++ to implicitly promote the BigUnsigned to a
 * BigInteger, but it seems to prefer converting the -5 to a BigUnsigned, which
 * causes an error.  If I take out constructors for BigUnsigned from signed
 * primitive integers, the BigUnsigned(3) becomes ambiguous, and if I take out
 * all the constructors but BigUnsigned(unsigned long), g++ uses that
 * constructor and gets a wrong (positive) answer.  Thus, I think we'll just
 * have to live with this cast. */
BigInteger p2 = BigInteger(BigUnsigned(3)) * -5;
TEST(p2); //-15

// === Test some previous bugs ===

{
	/* Test that BigInteger division sets the sign to zero.
	 * Bug reported by David Allen. */
	BigInteger num(3), denom(5), quotient;
	num.divideWithRemainder(denom, quotient);
	check(quotient);
	num = 5;
	num.divideWithRemainder(denom, quotient);
	check(num);
}

{
	/* Test that BigInteger subtraction sets the sign properly.
	 * Bug reported by Samuel Larkin. */
	BigInteger zero(0), three(3), ans;
	ans = zero - three;
	TEST(check(ans).getSign()); //-1
}

{
	/* Test that BigInteger multiplication shifts bits properly on systems
	 * where long is bigger than int.  (Obviously, this would only catch the
	 * bug when run on such a system.)
	 * Bug reported by Mohand Mezmaz. */
	BigInteger f=4; f*=3;
	TEST(check(f)); //12
}

{
	/* Test that bitwise XOR allocates the larger length.
	 * Bug reported by Sriram Sankararaman. */
	BigUnsigned a(0), b(3), ans;
	ans = a ^ b;
	TEST(ans); //3
}

{
	/* Test that an aliased multiplication works.
	 * Bug reported by Boris Dessy. */
	BigInteger num(5);
	num *= num;
	TEST(check(num)); //25
}

{
	/* Test that BigUnsignedInABase(std::string) constructor rejects digits
	 * too big for the specified base.
	 * Bug reported by Niakam Kazemi. */
	TEST(BigUnsignedInABase("f", 10)); //error
}

 {
   /* verify that divideWithRemainder throws an exception if c == q */
   BigInteger a(1), b(2);
   int result = 0;
   try {
     a.divideWithRemainder(b, a);
     result = 33;
   } catch (const char* foo) {
     result = 37;
   }
   TEST(result); //37
 }

} catch (const char *err) {
	cout << "UNCAUGHT ERROR: " << err << endl;
}

return 0;
}
