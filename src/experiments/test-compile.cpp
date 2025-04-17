
#include <glm/glm.hpp>
#include <cstdio>

#define SDF_HEADLESS true

struct bitfield{
    uint32_t a:16;
    uint32_t b:16;
};

template<typename T>
struct wrapper{
    uint8_t data[sizeof(T)];
    operator T&(){return *(T*)this;}
    wrapper(const T& r){*(T*)this=r;}
    T* operator->(){return (T*)this;}
};

#pragma omp declare target
static wrapper<bitfield> a = bitfield{4,6}; //This is ok
//static bitfield b = {5,6};                  //This one fails
#pragma omp end declare target

int main(){
    #pragma omp target
    {   
        printf("%d %d",a->a, a->b);
        printf("%f",glm::fract(1.0f));
    }
    return 0;
}