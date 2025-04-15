#include <cassert>

#define SDF_HEADLESS true
#include "sdf/sdf.hpp"

void test(auto sdf, float target){
    float sample_host = sdf.sample({0,0,0}), sample_target;
    #pragma omp target map(tofrom: sample_target)
    {
        sample_target=sdf.sample({0,0,0});
        printf("1: %f\n",sample_target);
    }
    printf("2: %f\n",sample_target);

    assert(abs(sample_host-(target))<sdf::EPS);
    assert(abs(sample_target-(target))<sdf::EPS);
}

int main(){
    {
        using namespace sdf::comptime;
        test(Sphere({5.0}),-5.0f);
        test(Sphere_t<sdf::color_attrs>({5.0}),-5.0f);
    }

    return 0;
}