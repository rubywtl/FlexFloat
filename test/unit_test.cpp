#include "gtest/gtest.h"
#include "CustomFP.hpp"  // Replace with the actual header containing your namespace

using namespace CustomFP;

// Check if the constructed custom fp has the right data width
TEST(FPTest, BitWidthInitTest) {
    ExMy fp(1,5,10);
    EXPECT_EQ(fp.get_sign_bits(), 1);
    EXPECT_EQ(fp.get_mantissa_bits(), 10);
    EXPECT_EQ(fp.get_exponent_bits(), 5);
    EXPECT_EQ(fp.get_total_bits(), 16);
}

// Check if the value saved in the object is the same as what we saved
TEST(FPTest, SetandGetRawBitsTest){
    // 0x4555 -> 0 10001 0101010101
    ExMy fp(1,5,10);
    fp.set_bits(0x4555);
    EXPECT_EQ(fp.exponent, 17);
    EXPECT_EQ(fp.mantissa, 341);
    EXPECT_EQ(fp.sign, 0);
    EXPECT_EQ(fp.get_raw_bits(), 0x4555);
}

TEST(FPTest, DataFormatCmpTest){

}

TEST(FPTest, AlignTest){
    
}

// TEST(FPTest, CheckIEEE754Status){
//     // 0x4555 -> 0 10001 0101010101
//     ExMy fp(1,5,10);
//     fp.set_bits(0x4555);
//     EXPECT_EQ(fp.status, 17);
// }

// Addition Tests
// - No overflow FP16 + FP16 = FP32
TEST(FPTest, AddNoOverflowTest) {
    ExMy* a = new ExMy(1, 3, 4);
    a->set_bits(0x30);
    ExMy* b = new ExMy(1, 3, 4);
    b->set_bits(0x28);
    ExMy* result = new ExMy(1, 5, 10);
    result->set_bits(0x0000);

    EXPECT_EQ(a->get_raw_bits(), 0x30);
    EXPECT_EQ(b->get_raw_bits(), 0x28);

    Adder adder;
    bool success = adder.add(a, b, result);
    EXPECT_EQ(success, true);
    EXPECT_EQ(result->get_raw_bits(), 0x3c00);
    EXPECT_EQ(a->get_raw_bits(), 0x30); // shouldn't change
    EXPECT_EQ(b->get_raw_bits(), 0x28); // shouldn't change

    std::cout << "A: mant=" << a->mantissa << " exp=" << a->exponent << "\n";
    std::cout << "B: mant=" << b->mantissa << " exp=" << b->exponent << "\n";
    std::cout << "Result: mant=" << result->mantissa << " exp=" << result->exponent << "\n";


    delete a;
    delete b;
    delete result;
}

// - Overflow FP8 + FP8 = FP8

// - No overflow FP8 + FP8 = FP16


// Subtraction Tests

// Multiplication Tests

// Division Tests

