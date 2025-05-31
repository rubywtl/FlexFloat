#pragma once

#include <cmath>    
#include <iostream>
#include <stdlib.h>
#include <stdio.h>

namespace CustomFP{
class ExMy {
private:
    unsigned sign_bits;
    unsigned mantissa_bits;
    unsigned exponent_bits;

public:
    enum FP_status {
        normal = 0,
        subnormal,
        NaN,
        inf,
        zero
    };

    FP_status status;


    unsigned sign;
    unsigned long long mantissa;
    unsigned long long exponent;

    // constructor
    ExMy(unsigned sign_bits, unsigned exponent_bits, unsigned mantissa_bits);

    // convert to a double (approximation)
    double approximation() const;

    // getters
    constexpr unsigned get_sign_bits() const { return sign_bits; }
    constexpr unsigned get_mantissa_bits() const { return mantissa_bits; }
    constexpr unsigned get_exponent_bits() const { return exponent_bits; }
    constexpr unsigned get_total_bits() const {
        return sign_bits + mantissa_bits + exponent_bits;
    }

    unsigned long long get_raw_bits();

    void set_bits(unsigned long long raw_value);

    // determine floating-point status
    void IEEE754_status_update();

    void clamp_to_format();

};

// operator base class
class Operator {
public:
    // check exponent alignment
    bool check_alignment(const ExMy& a, const ExMy& b) const;

    // check if format matches
    bool data_format_cmp(const ExMy& a, const ExMy& b) const;

    // align exponent by shifting mantissa
    void align(ExMy* a, const ExMy* target);
};

// multiplication
class Multiplier : public Operator {
public:
    bool mul(const ExMy* a, const ExMy* b, ExMy* result);
};

// division
class Divider : public Operator {
public:
    bool divide(const ExMy* a, const ExMy* b, ExMy* result);
};

// addition
class Adder : public Operator {
public:
    bool add(ExMy* a, ExMy* b, ExMy* result);
};

// subtraction
class Subtractor : public Operator {
public:
    bool subtract(ExMy* a, ExMy* b, ExMy* result);
};


void print_fp(const ExMy& f, const char* label);

void print_raw_fp(const ExMy& f, const char* label);
}