#include "CustomFP.hpp"
#include <cmath>

namespace CustomFP {

// Constructor
ExMy::ExMy(unsigned sign_bits, unsigned exponent_bits, unsigned mantissa_bits)
    : sign_bits(sign_bits), mantissa_bits(mantissa_bits), exponent_bits(exponent_bits),
      sign(0), mantissa(0), exponent(0), status(normal) {}

// Conversion
double ExMy::approximation() const {
    double fraction = (status == subnormal || exponent == 0)
                    ? (mantissa / static_cast<double>(1ULL << mantissa_bits))
                    : (1.0 + mantissa / static_cast<double>(1ULL << mantissa_bits));
    int exp_bias = (1 << (exponent_bits - 1)) - 1;
    int exp = static_cast<int>(exponent) - exp_bias;
    double value = std::ldexp(fraction, exp);
    return sign ? -value : value;
}

// Status and format
void ExMy::IEEE754_status_update() {
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

void ExMy::clamp_to_format() {
    sign &= (1U << sign_bits) - 1;
    mantissa &= (1ULL << mantissa_bits) - 1;
    exponent &= (1ULL << exponent_bits) - 1;
}

unsigned long long ExMy::get_raw_bits(){
    unsigned long long result;
    result = ((unsigned long long)sign) << (exponent_bits + mantissa_bits);
    result |= ((exponent) << mantissa_bits);
    result |= mantissa;
    return result;
}

void ExMy::set_bits(unsigned long long raw_value){
    mantissa = raw_value & ((1ULL << mantissa_bits) - 1);
    exponent = (raw_value >> mantissa_bits) & ((1ULL << exponent_bits) - 1);
    sign = (raw_value >> (mantissa_bits + exponent_bits)) & 1ULL;
}

// Operator base
bool Operator::check_alignment(const ExMy& a, const ExMy& b) const {
    return a.exponent == b.exponent;
}

bool Operator::data_format_cmp(const ExMy& a, const ExMy& b) const {
    return (a.get_exponent_bits() == b.get_exponent_bits() &&
            a.get_mantissa_bits() == b.get_mantissa_bits() &&
            a.get_sign_bits() == b.get_sign_bits());
}

void Operator::align(ExMy* a, const ExMy* target) {
    if (target->exponent < a->exponent) {
        auto scale = a->exponent - target->exponent;
        a->mantissa >>= scale;
        a->exponent = target->exponent;
    } 
}

// Multiplier
bool Multiplier::mul(const ExMy* a, const ExMy* b, ExMy* result) {
    result->sign = a->sign ^ b->sign;
    result->mantissa = a->mantissa * b->mantissa;
    result->exponent = a->exponent + b->exponent;
    result->IEEE754_status_update();
    result->clamp_to_format();
    return true;
}

// Divider
bool Divider::divide(const ExMy* a, const ExMy* b, ExMy* result) {
    result->sign = a->sign ^ b->sign;
    result->mantissa = a->mantissa / b->mantissa;
    result->exponent = a->exponent - b->exponent;
    result->IEEE754_status_update();
    result->clamp_to_format();
    return true;
}

bool Adder::add(ExMy* a, ExMy* b, ExMy* result) {
    if (!data_format_cmp(*a, *b)) return false;

    // make safe copies of a and b
    CustomFP::ExMy a_copy = *a;
    CustomFP::ExMy b_copy = *b;

    unsigned mantissa_bits = a_copy.get_mantissa_bits();
    unsigned implicit_bit = (1ULL << mantissa_bits);

    // restore implicit leading 1 if normal
    if (a_copy.status == CustomFP::ExMy::normal)
        a_copy.mantissa |= implicit_bit;
    if (b_copy.status == CustomFP::ExMy::normal)
        b_copy.mantissa |= implicit_bit;

    // align mantissas by shifting the one with smaller exponent
    if (a_copy.exponent > b_copy.exponent) {
        align(&b_copy, &a_copy);
        result->exponent = a_copy.exponent;
    } else {
        align(&a_copy, &b_copy);
        result->exponent = b_copy.exponent;
    }

    // add or subtract mantissas based on sign
    if (a_copy.sign == b_copy.sign) {
        result->mantissa = a_copy.mantissa + b_copy.mantissa;
        result->sign = a_copy.sign;

        // handle carry/overflow
        if (result->mantissa >> (mantissa_bits + 1)) {
            result->mantissa >>= 1;
            result->exponent += 1;
        }
    } else {
        if (a_copy.mantissa >= b_copy.mantissa) {
            result->mantissa = a_copy.mantissa - b_copy.mantissa;
            result->sign = a_copy.sign;
        } else {
            result->mantissa = b_copy.mantissa - a_copy.mantissa;
            result->sign = b_copy.sign;
        }
    }

    result->IEEE754_status_update();  // set normal/subnormal/zero status
    result->clamp_to_format();        // trim bits and remove implicit 1 if needed
    return true;
}

// Subtractor
bool Subtractor::subtract(ExMy* a, ExMy* b, ExMy* result) {
    if (!data_format_cmp(*a, *b)) return false;
    align(a, result);
    align(b, result);
    result->exponent = a->exponent;

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

// Utility
void print_fp(const ExMy& f, const char* label) {
    std::cout << label << ": "
              << (f.sign ? "-" : "+")
              << f.approximation() << " (status: " << f.status << ")\n";
}

void print_raw_fp(const ExMy& f, const char* label) {
    std::cout << label << ": "
              << f.sign << " "
              << f.exponent << " "
              << f.mantissa << "\n";
}

} // namespace CustomFP
