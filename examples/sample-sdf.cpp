#define SDF_HEADLESS
#include <enamento/sdf/sdf.hpp>
#include <glm/glm.hpp>
#include <iostream>

using namespace sdf::comptime;
using namespace glm;

int main(){
    auto sdf_0 = Sphere({1, {.idx=1}});
    auto sdf_base = SmoothJoin(Forward(sdf_0),(Sphere({1.5, {.idx=2}})+vec3{2.0,0.0,1.0}),{0.5f})+(Box({{1.0,2.0,3.0}, {.idx=1}})+vec3{2.0,5.0,1.0}) + Plane({{.gid=511,.idx=3}}) ; 
    auto sdf_rotated = sdf_base>vec3{0.0,0.0,-3.1415/8.0} ; 

    std::cout<<sdf_rotated({0.5f,0.2f,0.0f})<<"\n";

    return 0;
}