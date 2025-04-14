#pragma once

/**
 * @file sphere.hpp
 * @author karurochari
 * @brief Basic SDF for a sphere, centered at the origin. The 2D version is for a circle.
 * @date 2025-03-08
 * 
 * @copyright Copyright (c) 2025
 * 
 */


#ifndef SDF_INTERNALS
#error Don't import manually, this can only be used internally by the library
#endif

#include <cmath>
#include <cstddef>
#include <sstream>

#include "../sdf.hpp"
#include "../tree.hpp"


namespace sdf{

    namespace configs{
    }

    namespace{namespace impl_base{

        template <typename Attrs=default_attrs>
        struct Sphere{
            using attrs_t = Attrs;
            [[no_unique_address]] Attrs::extras_t cfg;
            float radius;

            constexpr Sphere(float radius, Attrs::extras_t cfg={}):cfg(cfg),radius(radius){}
            constexpr Sphere(Attrs::extras_t cfg={}):cfg(cfg){}

            constexpr inline float sample(const glm::vec3& pos)const {return glm::length(pos)-radius;}

            constexpr inline void traits(traits_t& to) const{
                PRIMITIVE_TRAIT_SYM;
                PRIMITIVE_TRAIT_GOOD;
                to.outer_box={{-radius,-radius,-radius,},{radius,radius,radius}};
            }

            constexpr inline static const char* _name = "Sphere";

            constexpr inline static field_t _fields[] = {
                FIELD(Sphere,float,deftype,radius, "Radius",float{0},float{INFINITY},float{5},nullptr),
            };

            PRIMITIVE_COMMONS
        };
    }}

    sdf_register_primitive(Sphere);
}

