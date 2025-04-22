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
#error "Don't import manually, this can only be used internally by the library"
#endif

#include "../sdf.hpp"

namespace sdf{

    namespace configs{
        struct Translate{
            glm::vec3 offset;
        };
    }

    namespace{namespace impl_base{

        template <typename L>
        struct Translate : utils::unary_op<L, configs::Translate>{
            using base = utils::unary_op<L, configs::Translate>;
            using base::base;

            constexpr float sample(const glm::vec3& pos) const{
                auto& left = base::left();
                auto lres = left.sample(pos-this->cfg.offset);
                return lres;
            }

            constexpr base::attrs_t operator()(const glm::vec3& pos) const{
                auto& left = base::left();
                auto lres = left(pos-this->cfg.offset);
                return lres;
            }

            constexpr inline void traits(const traits_t& from, const traits_t&, traits_t& to) const{
                to.is_sym={tribool::unknown,tribool::unknown,tribool::unknown};
                to.is_exact_inner=from.is_exact_inner;
                to.is_exact_outer=from.is_exact_outer;
                to.is_bounded_inner=from.is_bounded_inner;
                to.is_bounded_outer=from.is_bounded_outer;
                to.outer_box={from.outer_box.min-this->cfg.offset,from.outer_box.max-this->cfg.offset};
            }

            constexpr inline void traits(traits_t& to) const{
                traits_t ltraits;
                (base::left()).traits(ltraits);
                traits(ltraits,ltraits,to);
            }

            constexpr inline static const char* _name = "Translate";

            constexpr inline static field_t _fields[] = {
                FIELD_OP_R(Translate,vec3,deftype,offset, "Offset")
            };

            PRIMITIVE_NORMAL
        };
    }}

    sdf_register_operator_1(Translate);  

    namespace comptime {
        template <typename T> requires sdf_i<T> 
        constexpr inline auto operator + (const T& a, const glm::vec3& off){return Translate(a,{off});}
    }
    namespace polymorphic {
        template <typename T> requires sdf_i<T> 
        constexpr inline auto operator + (const T& a, const glm::vec3& off){return Translate(a,{off});}
    }
    namespace comptime_base {
        template <typename T> requires sdf_i<T> 
        constexpr inline auto operator + (const T& a, const glm::vec3& off){return Translate(a,{off});}
    }
    namespace polymorphic_base {
        template <typename T> requires sdf_i<T> 
        constexpr inline auto operator + (const T& a, const glm::vec3& off){return Translate(a,{off});}
    }
    namespace dynamic {
        template <typename T> requires sdf_i<T> 
        constexpr inline auto operator + (std::shared_ptr<T> a, const glm::vec3& off){return Translate(a,{off});}
    }


    //sdf_register_operator_2o(Common, *);
}