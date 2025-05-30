#include <cmath>    
#include <iostream>
#include <stdlib.h>
#include <stdio.h>

namespace CustomFP{
class ExMy {
public:
    enum FP_status {
        normal = 0,
        subnormal,
        NaN,
        inf,
        zero
    };

    FP_status status;

    unsigned sign_bits;
    unsigned mantissa_bits;
    unsigned exponent_bits;

    unsigned sign;
    unsigned long long mantissa;
    unsigned long long exponent;

    // constructor
    constexpr ExMy(unsigned sign_bits, unsigned mantissa_bits, unsigned exponent_bits)
        : sign_bits(sign_bits), mantissa_bits(mantissa_bits), exponent_bits(exponent_bits),
          sign(0), mantissa(0), exponent(0), status(normal) {}

    // convert to a double (approximation)
    double approximation() const {
        double fraction = (status == subnormal || exponent == 0)
                        ? (mantissa / static_cast<double>(1ULL << mantissa_bits))
                        : (1.0 + mantissa / static_cast<double>(1ULL << mantissa_bits));
        int exp_bias = (1 << (exponent_bits - 1)) - 1;
        int exp = static_cast<int>(exponent) - exp_bias;
        double value = std::ldexp(fraction, exp); // x * 2^d
        return sign ? -value : value;
    }

    // getters
    constexpr unsigned get_sign_bits() const { return sign_bits; }
    constexpr unsigned get_mantissa_bits() const { return mantissa_bits; }
    constexpr unsigned get_exponent_bits() const { return exponent_bits; }
    constexpr unsigned get_total_bits() const {
        return sign_bits + mantissa_bits + exponent_bits;
    }

    // determine floating-point status
    void IEEE754_status_update() {
        if (exponent == 0 && mantissa == 0)
            status = zero;
        else if (exponent == 0)
            status = subnormal;
        else if (exponent == ((1ULL << exponent_bits) - 1) && mantissa == 0)
            status = inf;
        else if (exponent == ((1ULL << exponent_bits) - 1) && mantissa != 0)
            status = NaN;
        else
            status = normal;
    }

    void clamp_to_format() {
        sign &= (1U << sign_bits) - 1;
        mantissa &= (1ULL << mantissa_bits) - 1;
        exponent &= (1ULL << exponent_bits) - 1;
    }

};

// operator base class
class Operator {
public:
    // check exponent alignment
    bool check_alignment(const ExMy& a, const ExMy& b) const {
        return a.exponent == b.exponent;
    }

    // check if format matches
    bool data_format_cmp(const ExMy& a, const ExMy& b) const {
        return (a.exponent_bits == b.exponent_bits &&
                a.mantissa_bits == b.mantissa_bits &&
                a.sign_bits == b.sign_bits);
    }

    // align exponent by shifting mantissa
    void align(ExMy* a, const ExMy* target) {
        if (target->exponent > a->exponent) {
            auto scale = target->exponent - a->exponent;
            a->mantissa <<= scale;
        } else if (target->exponent < a->exponent) {
            auto scale = a->exponent - target->exponent;
            a->mantissa >>= scale;
        }
        a->exponent = target->exponent;
    }
};

// multiplication
class multiplier : public Operator {
public:
    bool mul(const ExMy* a, const ExMy* b, ExMy* result) {
        result->sign = a->sign ^ b->sign;
        result->mantissa = a->mantissa * b->mantissa;
        result->exponent = a->exponent + b->exponent;
        result->IEEE754_status_update();
        result->clamp_to_format();

        return true;
    }
};

// division
class divider : public Operator {
public:
    bool divide(const ExMy* a, const ExMy* b, ExMy* result) {
        result->sign = a->sign ^ b->sign;
        result->mantissa = a->mantissa / b->mantissa;
        result->exponent = a->exponent - b->exponent;
        result->IEEE754_status_update();
        result->clamp_to_format();

        return true;
    }
};

// addition
class adder : public Operator {
public:
    bool add(ExMy* a, ExMy* b, ExMy* result) {
        if (!data_format_cmp(*a, *b)) return false;
        align(a, result);
        align(b, result);
        result->mantissa = a->mantissa + b->mantissa;
        result->exponent = a->exponent;

        // sign bit: magnitude check
        if (a->sign == b->sign) {
            result->mantissa = a->mantissa + b->mantissa;
            result->sign = a->sign;
        } else {
            if (a->mantissa > b->mantissa) {
                result->mantissa = a->mantissa - b->mantissa;
                result->sign = a->sign;
            } else {
                result->mantissa = b->mantissa - a->mantissa;
                result->sign = b->sign;
            }
        }

        result->IEEE754_status_update();
        result->clamp_to_format();

        return true;
    }
};

// subtraction
class subtraction : public Operator {
public:
    bool subtract(ExMy* a, ExMy* b, ExMy* result) {
        if (!data_format_cmp(*a, *b)) return false;
        align(a, result);
        align(b, result);
        result->mantissa = a->mantissa - b->mantissa;
        result->exponent = a->exponent;

        // sign bit: magnitude check
        // -> change b to -b
        ExMy b_neg = *b;
        b_neg.sign = !b->sign;
        if (a->sign == b_neg.sign) {
            result->mantissa = a->mantissa + b_neg.mantissa;
            result->sign = a->sign;
        } else {
            if (a->mantissa > b_neg.mantissa) {
                result->mantissa = a->mantissa - b_neg.mantissa;
                result->sign = a->sign;
            } else {
                result->mantissa = b_neg.mantissa - a->mantissa;
                result->sign = b_neg.sign;
            }
        }

        result->IEEE754_status_update();
        result->clamp_to_format();

        return true;
    }
};


// print current value
void print_fp(const ExMy& f, const char* label) {
    std::cout << label << ": "
                << (f.sign ? "-" : "+")
                << f.approximation() << " (status: " << f.status << ")\n";
}

void print_raw_fp(const ExMy& f, const char* label) {
    std::cout << label << ": "
            << (f.sign) << " "
            << f.exponent << " " << f.mantissa << "\n";
}

}
