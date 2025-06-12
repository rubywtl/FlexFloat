# ⚡ FlexFloat
FlexFloat was created to simulate floating point numerical algorithms as they are implemented in hardware, while providing a user-friendly programming and testing interface. The goal is to make it easy to experiment with and verify floating point behaviors outside traditional computer architecture contexts—such as in machine learning, scientific computing, and other numerical applications.

By allowing customizable bit-widths for sign, exponent, and mantissa, FlexFloat supports a wide range of precision requirements, enabling researchers and developers to explore alternative floating point formats with confidence.

The library fully supports core arithmetic operations and handles all IEEE 754 special cases, including NaN, infinity, zero, and subnormal numbers.

## ✅ Test & Verification Coverage

This library has been thoroughly verified through unit tests covering:

- ✔️ Bit-level correctness (raw IEEE 754 encoding/decoding)
- ✔️ Arithmetic operations (add, subtract, multiply, divide)
- ✔️ Special cases (NaN, Inf, Zero, Subnormal, Overflow, Divide-by-zero)

All core behaviors conform to IEEE 754 expectations.

## 🛠️ How To Use It
### Prerequisites  
```shell
sudo apt update
sudo apt --fix-missing install
sudo apt install g++
sudo apt install cmake
```

### Build & Test
build then test
```shell
mkdir build && cd build
cmake ..
make
ctest
ctest --output-on-failure
```

## 🚀 Applicatoins
To use FlexFloat in your projects, include the provided headers and link against the compiled library.
