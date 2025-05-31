#include "CustomFP.hpp" 
using namespace CustomFP;

class accumulator{
public:
    ExMy* fp16_result = new ExMy(1, 10, 5);
    Adder* adder;
    Multiplier* multiplier;

    void accumulate(ExMy* a, ExMy* b){
        ExMy* partial_result;
        multiplier->mul(a, b, partial_result);
        adder->add(partial_result, fp16_result, fp16_result);
    }
}

