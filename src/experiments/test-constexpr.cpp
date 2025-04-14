#include <iostream>

#define FLAG true


#if FLAG==true
    #define FIELD(x) x
#else
    #define FIELD(x) 
#endif

#define MACRO(X) \
struct X {\
    int member;\
    FIELD(int a);\
};\

int main(){
    #if FLAG==true
        MACRO(A)
    #else
        MACRO(B)
    #endif

    #if FLAG==true
        A test;
        test.member;
        test.a;
    #else
        B test;
        test.member;
    #endif
    return 0;
}