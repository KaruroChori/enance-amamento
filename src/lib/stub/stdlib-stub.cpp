/*
    This files contains stubs needed to facilitate adoption of C++ STL parts in offloaded devices.
    Even without libc++ nor exceptions, there is a lot which can be useful in there.
*/

#if __NVPTX__
#include <cstdio>
#pragma omp declare target
namespace std {
    //Enable assertions (this is for libc)
    //__attribute__((used)) extern "C" void __assert_fail(){fprintf(stderr,"Assertion failed");__trap();while(true);}
    //Enable std::function
    __attribute__((used)) void __throw_bad_function_call() {fprintf(stderr,"Bad function call");__trap();while(true);}
    //Enable std::string
    __attribute__((used)) void __throw_length_error(const char* msg) {fprintf(stderr,"Length error: %s",msg);__trap();while(true);}
    //Enable several libc++ things
    __attribute__((used)) void __throw_bad_alloc() {fprintf(stderr,"Bad allocation");__trap();while(true);}
}
#pragma omp end declare target
#endif

#if __AMDGCN__
namespace std {
    //Or maybe __builtin_trap? I need to check if its behaviour is consistent.
    void __throw_bad_function_call() {asm volatile("s_trap");}    //Needed for std::function to work in general cases
}
#endif