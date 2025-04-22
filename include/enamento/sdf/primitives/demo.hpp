#pragma once

/**
 * @file demo.hpp
 * @author karurochari
 * @brief A demo SDF (just a sphere) to demo widgets in the UI configuration.
 * @date 2025-03-08
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef SDF_INTERNALS
#error "Don't import manually, this can only be used internally by the library"
#endif

#include "utils/tribool.hpp"
#include <cmath>

#include "../sdf.hpp"
#include "../tree.hpp"


namespace sdf{

    namespace configs{
    }

    namespace{namespace impl_base{

        template <typename Attrs=default_attrs>
        struct Demo{
            using attrs_t = Attrs;
            [[no_unique_address]] Attrs::extras_t cfg;
            float radius;
            vec3 vec3_v;
            vec2 vec2_v;
            ivec3 ivec3_v;
            tribool tbool;

            constexpr Demo(float radius, Attrs::extras_t cfg={}):cfg(cfg),radius(radius){}

            constexpr inline float sample(const glm::vec3& pos)const {return glm::length(pos)-radius;}

            constexpr inline void traits(traits_t& to) const{
                PRIMITIVE_TRAIT_SYM;
                PRIMITIVE_TRAIT_GOOD;
                to.outer_box={vec3(-INFINITY),vec3(INFINITY)};
            }
 
            constexpr inline static const char* _name = "Demo";

            constexpr inline static field_t _fields[] = {
                FIELD(Demo,float,deftype,radius, "Radius",0,12,5,nullptr),
                FIELD(Demo,vec3,deftype,vec3_v, "glm::vec3",vec3(0.1f,0.2f,0.3f),vec3(1.0f,1.0f,1.0f),vec3(0.5f,0.5f,0.5f),nullptr),
                FIELD(Demo,vec2,deftype,vec2_v, "glm::vec2",vec2(0.1f,-0.2f),vec2(1.0f,1.0f),vec2(0.5f,0.5f),nullptr),
                FIELD(Demo,ivec3,deftype,ivec3_v, "glm::ivec3",vec3(-5,-5,-5),vec3(5,5,5),vec3(0,0,0),nullptr),
                FIELD_R(Demo,tribool,deftype,tbool, "tbool"),
            };

            PRIMITIVE_COMMONS
        };
    }}

    sdf_register_primitive(Demo);
}