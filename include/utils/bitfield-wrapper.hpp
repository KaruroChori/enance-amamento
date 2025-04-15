#pragma once

#include <cstdint>

/*
WORKAROUND FOR https://github.com/llvm/llvm-project/issues/132342
constexpr failing as reinterpreted_cast is not allowed in constexpr.
*/
template<typename T>
struct bfw{
    uint8_t data[sizeof(T)];
    operator T&(){return *(T*)this;}
    bfw(const T& r){*(T*)this=r;}
    T* operator->(){return (T*)this;}
};
