#include "omp.h"

#define SDF_HEADLESS true

#define GLM_FORCE_SWIZZLE
#define GLM_FORCE_INLINE

#include <glm/ext/vector_float3.hpp>
#include <iostream>
#include <sdf/sdf.hpp>
#include <ui/ui.hpp>

using namespace sdf;

struct tw{
    int i = 0;
}v;

namespace test1{
    using namespace sdf::comptime;
    float test(){
        auto d =  Sphere({7.0});
        auto t = Sphere({5.0}) + Sphere({6.0}) + Forward(d);
        d.radius=100;
        tree::builder N1;
        N1.close(t.to_tree(N1));
        auto w = N1.build();
        //return sdf::utils::root<sdf::default_attrs>(w)({0.5,0.5}).distance;
        return t({0.5,0.5,0.0}).distance;
    }
}

namespace test2{
    using namespace sdf::polymorphic;
    float test(){
        auto d =  Sphere({7.0});
        auto t = Sphere({5.0}) + Sphere({6.0}) + Forward(d);
        d.radius=100;
        return t({0.5,0.5,0.0}).distance;
    }
}

namespace test3{
    using namespace sdf::dynamic;
    float test(){
        auto d =  Sphere({7.0});
        auto t = Sphere({5.0}) + Sphere({6.0}) + Forward(d);  //Not needed, but implemented just to allow reuse of similar code.
        ((Sphere_t<default_attrs>&)(*d)).radius=100;
        return (*t)({0.5,0.5,0.0}).distance;
    }
}

/*
namespace test4{
    using namespace sdf::tree;
    float test(){
        uint8_t buffer[256];
        sdf::utils::tree_idx_base::base=buffer;
        auto a =  sdf::utils::tree_idx<Sphere_t<sdf::default_attrs>>{0};
        a->radius=5;
        auto b =  sdf::utils::tree_idx<Sphere_t<sdf::default_attrs>>{10};
        b->radius=10;
        auto t = a+b;
        return t({0.6,0.6}).distance;
    }
}
*/

int main(){
    /*sdl_instance sdli;
    App app(800,600);
    app.run({+[](const App::camera_t& camera, void* buffer){
        return 0;
    }, +[](const App::camera_t& camera, const glm::vec2& point){return glm::vec3{0,0,0};}});*/
    std::cout<<test1::test()<<" ---\n";
    std::cout<<test2::test()<<" ---\n";
    std::cout<<test3::test()<<" ---\n";
    //std::cout<<test4::test()<<" ---\n";

    /*Sphere_t<default_attrs> S1{{},5.0}, S2{{},10.0}, S3{{},101.0};
    S1({0,1});
    auto expr = Join(Join(S1,S2),S3);
    expr({15,6});
    std::cout<<"Hello\n";
    {
    polymorphic::Sphere_t<default_attrs> dS1({{},0.5});
    auto S4 = dynamic::Sphere({{},5.0});//({{},5.0})
    auto t = dynamic::Sphere({{},0.5});
    dynamic::SmoothJoin(t,dynamic::Sphere({{},10.5}), {.v=22});
    dS1({1,11.0});
    S4->operator()({5,6.7});
    }*/
}