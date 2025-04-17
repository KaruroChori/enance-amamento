#pragma once

/**
 * @file join.hpp
 * @author karurochari
 * @brief Common, or Intersection. Boolean operation with harsh edges.
 * @date 2025-03-08
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef SDF_INTERNALS
#error Don't import manually, this can only be used internally by the library
#endif

#include "../sdf.hpp"

namespace sdf{

    namespace configs{
        struct Rotate{
            glm::vec3 rotation;
        };
    }

    namespace{namespace impl_base{

        template <typename L>
        struct Rotate : utils::unary_op<L, configs::Rotate>{
            using base = utils::unary_op<L, configs::Rotate>;
            using base::base;


            constexpr static inline mat3 rotate_x(float a){float sa = sin(a); float ca = cos(a); return mat3(vec3(1.,.0f,.0f),    vec3(.0f,ca,sa),    vec3(.0f,-sa,ca));}
            constexpr static inline mat3 rotate_y(float a){float sa = sin(a); float ca = cos(a); return mat3(vec3(ca,.0f,sa),     vec3(.0f,1.,.0f),   vec3(-sa,.0f,ca));}
            constexpr static inline mat3 rotate_z(float a){float sa = sin(a); float ca = cos(a); return mat3(vec3(ca,sa,.0f),     vec3(-sa,ca,.0f),   vec3(.0f,.0f,1.f));}

            constexpr float sample(const glm::vec3& pos) const{
                auto& left = base::left();
                auto newpos=pos;
                newpos=newpos*rotate_x(this->cfg.rotation.x);
                newpos=newpos*rotate_y(this->cfg.rotation.y);
                newpos=newpos*rotate_z(this->cfg.rotation.z);
                auto lres = left.sample(newpos);
                return lres;
            }

            constexpr base::attrs_t operator()(const glm::vec3& pos) const{
                auto& left = base::left();
                auto newpos=pos;
                newpos=newpos*rotate_x(this->cfg.rotation.x);
                newpos=newpos*rotate_y(this->cfg.rotation.y);
                newpos=newpos*rotate_z(this->cfg.rotation.z);
                auto lres = left(newpos);
                return lres;
            }

            constexpr inline void traits(const traits_t& from, const traits_t&, traits_t& to) const{
                to.is_sym={tribool::unknown,tribool::unknown,tribool::unknown};
                to.is_exact_inner=from.is_exact_inner;
                to.is_exact_outer=from.is_exact_outer;
                to.is_bounded_inner=from.is_bounded_inner;
                to.is_bounded_outer=from.is_bounded_outer;
                to.outer_box=cbbox(from.outer_box);
            }

            constexpr inline void traits(traits_t& to) const{
                traits_t ltraits;
                (base::left()).traits(ltraits);
                traits(ltraits,ltraits,to);
            }

            constexpr inline bbox_t cbbox(bbox_t box) const{
                box.min = box.min*rotate_x(this->cfg.rotation.x);
                box.min = box.min*rotate_y(this->cfg.rotation.y);
                box.min = box.min*rotate_z(this->cfg.rotation.z);

                box.max = box.max*rotate_x(this->cfg.rotation.x);
                box.max = box.max*rotate_y(this->cfg.rotation.y);
                box.max = box.max*rotate_z(this->cfg.rotation.z);

                return box;
            }

            constexpr inline static const char* _name = "Rotate";

            constexpr inline static field_t _fields[] = {
                FIELD_OP_R(Rotate,vec3,deftype,rotation, "Rotation (X,Y,Z) in rad")
            };

            PRIMITIVE_NORMAL
        };
    }}

    sdf_register_operator_1(Rotate);  

    namespace comptime {
        template <typename T> requires sdf_i<T> 
        constexpr inline auto operator > (const T& a, const glm::vec3& off){return Rotate(a,{off});}
    }
    namespace polymorphic {
        template <typename T> requires sdf_i<T> 
        constexpr inline auto operator > (const T& a, const glm::vec3& off){return Rotate(a,{off});}
    }
    namespace comptime_base {
        template <typename T> requires sdf_i<T> 
        constexpr inline auto operator > (const T& a, const glm::vec3& off){return Rotate(a,{off});}
    }
    namespace polymorphic_base {
        template <typename T> requires sdf_i<T> 
        constexpr inline auto operator > (const T& a, const glm::vec3& off){return Rotate(a,{off});}
    }
    namespace dynamic {
        template <typename T> requires sdf_i<T> 
        constexpr inline auto operator > (std::shared_ptr<T> a, const glm::vec3& off){return Rotate(a,{off});}
    }


    //sdf_register_operator_2o(Common, *);
}