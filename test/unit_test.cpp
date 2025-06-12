#include "gtest/gtest.h"
#include "CustomFP.hpp"
#include <iostream>

using namespace CustomFP;

// Test Summary
// - Basic Initialization Tests: raw bit setting, IEEE 754 special case tags
// - Basic Operator Tests: alignment, format compare
// - Arithmetic Tests: single precision + mixed precision computations
// - Arithmetic Edge Case Tests: NaN, inf, multiply by zero, subnormal, divide by zero



// ------------------------------------------------------------
// 1. Initialization & Bit Manipulation Tests
// ------------------------------------------------------------

TEST(FPTest, BitWidthInitTest) {
    ExMy fp(1, 5, 10);
    EXPECT_EQ(fp.get_sign_bits(), 1);
    EXPECT_EQ(fp.get_mantissa_bits(), 10);
    EXPECT_EQ(fp.get_exponent_bits(), 5);
    EXPECT_EQ(fp.get_total_bits(), 16);
}

TEST(FPTest, SetandGetRawBitsTest) {
    ExMy fp(1, 5, 10);
    fp.set_bits(0x4555);
    EXPECT_EQ(fp.exponent, 17);
    EXPECT_EQ(fp.mantissa, 341);
    EXPECT_EQ(fp.sign, 0);
    EXPECT_EQ(fp.get_raw_bits(), 0x4555);
    EXPECT_EQ(fp.get_flag(), ExMy::FP_status::normal);
    std::cout << fp.get_flag_str() << std::endl;
}

TEST(FPTest, SetandGetRawBits_Subnormal_Test) {
    ExMy fp(1, 5, 10);
    fp.set_bits(0x02AA);  // 0000 0010 1010 1010
    EXPECT_EQ(fp.exponent, 0);               // exponent = 0 → subnormal
    EXPECT_EQ(fp.mantissa, 0x2AA);           // mantissa = 0x2AA
    EXPECT_EQ(fp.sign, 0);                   // sign = 0
    EXPECT_EQ(fp.get_raw_bits(), 0x02AA);
    EXPECT_EQ(fp.get_flag(), ExMy::FP_status::subnormal);
    std::cout << fp.get_flag_str() << std::endl;
}

TEST(FPTest, SetandGetRawBits_NaN_Test) {
    ExMy fp(1, 5, 10);
    fp.set_bits(0x7FC1);  // 0111 1111 1100 0001
    EXPECT_EQ(fp.exponent, 0x1F);            // exponent = 0x1F (max for 5-bit)
    EXPECT_NE(fp.mantissa, 0);               // mantissa ≠ 0
    EXPECT_EQ(fp.sign, 0);
    EXPECT_EQ(fp.get_raw_bits(), 0x7FC1);
    EXPECT_EQ(fp.get_flag(), ExMy::FP_status::NaN);
    std::cout << fp.get_flag_str() << std::endl;
}

TEST(FPTest, SetandGetRawBits_PositiveInf_Test) {
    ExMy fp(1, 5, 10);
    fp.set_bits(0x7C00);  // 0111 1100 0000 0000
    EXPECT_EQ(fp.exponent, 0x1F);            // exponent = 0x1F
    EXPECT_EQ(fp.mantissa, 0);               // mantissa = 0
    EXPECT_EQ(fp.sign, 0);                   // sign = 0
    EXPECT_EQ(fp.get_raw_bits(), 0x7C00);
    EXPECT_EQ(fp.get_flag(), ExMy::FP_status::inf);
    std::cout << fp.get_flag_str() << std::endl;
}

TEST(FPTest, SetandGetRawBits_NegativeInf_Test) {
    ExMy fp(1, 5, 10);
    fp.set_bits(0xFC00);  // 1111 1100 0000 0000
    EXPECT_EQ(fp.exponent, 0x1F);            // exponent = 0x1F
    EXPECT_EQ(fp.mantissa, 0);               // mantissa = 0
    EXPECT_EQ(fp.sign, 1);                   // sign = 1
    EXPECT_EQ(fp.get_raw_bits(), 0xFC00);
    EXPECT_EQ(fp.get_flag(), ExMy::FP_status::inf);
    std::cout << fp.get_flag_str() << std::endl;
}

TEST(FPTest, SetandGetRawBits_PositiveZero_Test) {
    ExMy fp(1, 5, 10);
    fp.set_bits(0x0000);  // All bits zero
    EXPECT_EQ(fp.exponent, 0);
    EXPECT_EQ(fp.mantissa, 0);
    EXPECT_EQ(fp.sign, 0);
    EXPECT_EQ(fp.get_raw_bits(), 0x0000);
    EXPECT_EQ(fp.get_flag(), ExMy::FP_status::zero);
    std::cout << fp.get_flag_str() << std::endl;
}


// ------------------------------------------------------------
// 2. Functional/Utility Tests
// ------------------------------------------------------------

TEST(FPTest, DataFormatCMP_Test) {
    ExMy a1(1, 3, 4);
    ExMy a2(1, 3, 4);
    ExMy a3(1, 4, 3);
    Operator op;

    EXPECT_TRUE(op.data_format_cmp(a1, a2));
    EXPECT_FALSE(op.data_format_cmp(a2, a3));
}


// ------------------------------------------------------------
// 3. Addition Tests
// ------------------------------------------------------------

// FP8 + FP8 = FP16
TEST(FPTest, Add_MixedPrecisionTest) {
    auto a = std::make_unique<ExMy>(1, 3, 4);
    auto b = std::make_unique<ExMy>(1, 3, 4);
    auto result = std::make_unique<ExMy>(1, 5, 10);

    a->set_bits(0x30);
    b->set_bits(0x30);
    result->set_bits(0x0000);

    Adder adder;
    bool success = adder.add(a.get(), b.get(), result.get());

    EXPECT_TRUE(success);
    EXPECT_EQ(result->get_raw_bits(), 0x4000);
    EXPECT_EQ(a->get_raw_bits(), 0x30);
    EXPECT_EQ(b->get_raw_bits(), 0x30);
}

// FP8 + FP8 = FP8
TEST(FPTest, E3M4_Add_SamePrecision_Test) {
    auto a = std::make_unique<ExMy>(1, 3, 4);
    auto b = std::make_unique<ExMy>(1, 3, 4);
    auto result = std::make_unique<ExMy>(1, 3, 4);

    a->set_bits(0x34);
    b->set_bits(0x34);
    result->set_bits(0x00);

    Adder adder;
    bool success = adder.add(a.get(), b.get(), result.get());

    EXPECT_TRUE(success);
    EXPECT_EQ(result->get_raw_bits(), 0x44);
    EXPECT_EQ(a->get_raw_bits(), 0x34);
    EXPECT_EQ(b->get_raw_bits(), 0x34);
}

// FP16 + FP16 = FP16
TEST(FPTest, FP16_Add_NoOverflow_SamePrecision_Test) {
    auto a = std::make_unique<ExMy>(1, 5, 10);
    auto b = std::make_unique<ExMy>(1, 5, 10);
    auto result = std::make_unique<ExMy>(1, 5, 10);

    a->set_bits(0x4A40);
    b->set_bits(0x4A40);
    result->set_bits(0x4E40);

    Adder adder;
    bool success = adder.add(a.get(), b.get(), result.get());

    EXPECT_TRUE(success);
    EXPECT_EQ(result->get_raw_bits(), 0x4E40);
    EXPECT_EQ(a->get_raw_bits(), 0x4A40);
    EXPECT_EQ(b->get_raw_bits(), 0x4A40);
}

TEST(FPTest, Add_Inf_Test) {
    auto a = std::make_unique<ExMy>(1, 5, 10);
    auto b = std::make_unique<ExMy>(1, 5, 10);
    auto result = std::make_unique<ExMy>(1, 5, 10);

    // a = +Inf (sign=0, exponent=all 1s, mantissa=0)
    a->set_bits(0x7C00);
    // b = finite number (say, 3.25 -> 0x4540 in FP16)
    b->set_bits(0x4540);
    result->set_bits(0x0000);

    Adder adder;
    bool success = adder.add(a.get(), b.get(), result.get());

    EXPECT_TRUE(success);
    EXPECT_EQ(result->get_raw_bits(), 0x7C00);  // result should be +Inf
    EXPECT_EQ(result->get_flag(), ExMy::FP_status::inf);
}


TEST(FPTest, Add_NaN_Test) {
    auto a = std::make_unique<ExMy>(1, 5, 10);
    auto b = std::make_unique<ExMy>(1, 5, 10);
    auto result = std::make_unique<ExMy>(1, 5, 10);

    // a = NaN (exponent=all 1s, mantissa≠0)
    a->set_bits(0x7E01);  // Example NaN in FP16
    // b = any finite value, e.g., 1.0 = 0x3C00
    b->set_bits(0x3C00);
    result->set_bits(0x0000);

    Adder adder;
    bool success = adder.add(a.get(), b.get(), result.get());

    EXPECT_TRUE(success);
    EXPECT_EQ(result->get_flag(), ExMy::FP_status::NaN);
}


// ------------------------------------------------------------
// 4. Multiplication Tests
// ------------------------------------------------------------

TEST(FPTest, MultiplicationWithOne) {
    auto a = std::make_unique<ExMy>(1, 5, 10);  // 0x3C00 = 1.0
    auto b = std::make_unique<ExMy>(1, 5, 10);  // 0x4A40 = 12.5
    auto result = std::make_unique<ExMy>(1, 8, 23);  // FP32 format

    a->set_bits(0x3C00);
    b->set_bits(0x4A40);
    result->set_bits(0x0000);

    Multiplier multiplier;
    multiplier.mul(a.get(), b.get(), result.get());

    EXPECT_EQ(result->get_raw_bits(), 0x41480000);  // 12.5 in FP32
    EXPECT_EQ(a->get_raw_bits(), 0x3C00);
    EXPECT_EQ(b->get_raw_bits(), 0x4A40);
}


TEST(FPTest, MultiplicationDifferentValues) {
    auto a = std::make_unique<ExMy>(1, 5, 10);  // 0x4800 = 8.0
    auto b = std::make_unique<ExMy>(1, 5, 10);  // 0x4400 = 4.0
    auto result = std::make_unique<ExMy>(1, 8, 23);  // FP32 format

    a->set_bits(0x4800);
    b->set_bits(0x4400);
    result->set_bits(0x0000);

    Multiplier multiplier;
    multiplier.mul(a.get(), b.get(), result.get());

    EXPECT_EQ(result->get_raw_bits(), 0x42000000);  // 32.0 in FP32
    EXPECT_EQ(a->get_raw_bits(), 0x4800);
    EXPECT_EQ(b->get_raw_bits(), 0x4400);
}


TEST(FPTest, MultiplicationWithZero) {
    auto a = std::make_unique<ExMy>(1, 5, 10);
    auto b = std::make_unique<ExMy>(1, 5, 10);
    auto result = std::make_unique<ExMy>(1, 8, 23);

    a->set_bits(0x0000);
    b->set_bits(0x4A40);
    result->set_bits(0x0000);

    Multiplier multiplier;
    multiplier.mul(a.get(), b.get(), result.get());

    EXPECT_EQ(result->get_raw_bits(), 0x0000);
    EXPECT_EQ(a->get_raw_bits(), 0x0000);
    EXPECT_EQ(b->get_raw_bits(), 0x4A40);
}

TEST(FPTest, Multiply_Inf_Test) {
    auto a = std::make_unique<ExMy>(1, 5, 10);
    auto b = std::make_unique<ExMy>(1, 5, 10);
    auto result = std::make_unique<ExMy>(1, 5, 10);

    // a = +Inf (sign = 0, exp = all 1s, mantissa = 0)
    a->set_bits(0x7C00);
    // b = 2.0 = 0x4000 in FP16
    b->set_bits(0x4000);
    result->set_bits(0x0000);

    Multiplier multiplier;
    bool success = multiplier.mul(a.get(), b.get(), result.get());

    EXPECT_TRUE(success);
    EXPECT_EQ(result->get_flag(), ExMy::FP_status::inf);
    EXPECT_EQ(result->get_raw_bits(), 0x7C00);
}


TEST(FPTest, Multiply_NaN_Test) {
    auto a = std::make_unique<ExMy>(1, 5, 10);
    auto b = std::make_unique<ExMy>(1, 5, 10);
    auto result = std::make_unique<ExMy>(1, 5, 10);

    // a = NaN (exp = all 1s, mantissa ≠ 0)
    a->set_bits(0x7E01);  // Quiet NaN
    // b = 1.0 = 0x3C00
    b->set_bits(0x3C00);
    result->set_bits(0x0000);

    Multiplier multiplier;
    bool success = multiplier.mul(a.get(), b.get(), result.get());

    EXPECT_TRUE(success);
    EXPECT_EQ(result->get_flag(), ExMy::FP_status::NaN);
}


// ------------------------------------------------------------
// 5. Subtraction Tests
// ------------------------------------------------------------


// ------------------------------------------------------------
// 6. Division Tests
// ------------------------------------------------------------


// ------------------------------------------------------------
// 7. Special Case Tests
// ------------------------------------------------------------


