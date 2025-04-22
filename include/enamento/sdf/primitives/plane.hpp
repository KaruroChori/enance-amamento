#pragma once

/**
 * @file plane.hpp
 * @author karurochari
 * @brief Basic SDF for a plane, covering the half negative plane.
 * @date 2025-03-14
 * 
 * @copyright Copyright (c) 2025
 * 
 */


#ifndef SDF_INTERNALS
#error "Don't import manually, this can only be used internally by the library"
#endif

#include <cmath>

#include "../sdf.hpp"

namespace sdf{

    namespace configs{
    }

    namespace{namespace impl_base{

        template <typename Attrs=default_attrs>
        struct Plane{
            using attrs_t = Attrs;
            [[no_unique_address]] Attrs::extras_t cfg;

            constexpr inline float sample(const glm::vec3& pos)const {return dot(pos,vec3(0,1,0));}

            constexpr Plane(Attrs::extras_t cfg={}):cfg(cfg){}

            constexpr inline void traits(traits_t& to) const{
                to.is_sym={true,false,true};
                PRIMITIVE_TRAIT_GOOD;
                to.outer_box={{-INFINITY,-INFINITY,-INFINITY,},{INFINITY,0,INFINITY}};
            }

            constexpr inline static const char* _name = "Plane";

            constexpr inline static field_t _fields[] = {};

            PRIMITIVE_COMMONS
        };
    }}

    sdf_register_primitive(Plane);
}