
#pragma once

/**
 * @file forward.hpp
 * @author karurochari
 * @brief Helper to reuse children ensuring they are referenced and not copied.
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
    }

    namespace{namespace impl_base{

        template <typename Attrs, template<typename, typename... Args> typename Src, typename... Args>
        struct Forward{
            using attrs_t = Attrs;
            const Src<Attrs,Args...>& src;

            constexpr Forward(const Src<Attrs,Args...>& ref):src(ref){}

            constexpr inline float sample(const glm::vec3& pos)const {return src.sample(pos);}
            constexpr inline Attrs operator()(const glm::vec3& pos)const {return src.operator()(pos);}

            constexpr inline void traits(traits_t& t) const{return src.traits(t); };

            constexpr inline static field_t _fields[] = {};
        };
  
    }}

    namespace{ namespace impl{
        template <typename Attrs, template<typename, typename... Args> typename Src, typename... Args>
        struct Forward : impl_base::Forward<Attrs,Src,Args...>{
            using impl_base::Forward<Attrs,Src,Args...>::Forward;
            using typename impl_base::Forward<Attrs,Src,Args...>::attrs_t;

            uint16_t serialize(tree::builder& dst)const {
                //TODO: Possibly revise this one. For now it copies.
                return this->src.serialize(dst);
            }

            bool to_cpp(ostream& dst) const{
                #if SDF_IS_HOST==true
                    dst<<std::format("Forward(node_{})",(void*)&(this->src));
                    return true;
                #else
                    return false;
                #endif
            }
    
            bool to_xml(xml& dst) const{
                return false;
            }
        };

  
        //In order not to disappear into nothingness, the reference must be indirectly referenced.
        template <typename Attrs>
        struct ForwardDynamic : utils::base_dyn<Attrs>{
            using attrs_t = Attrs;
            const std::shared_ptr<utils::base_dyn<Attrs>>& src;

            constexpr ForwardDynamic(const std::shared_ptr<utils::base_dyn<Attrs>>& ref):src(ref){}

            constexpr inline Attrs operator()(const glm::vec3& pos)const final{return src->operator()(pos);}
            constexpr inline float sample(const glm::vec3& pos)const final{return src->sample(pos);}

            constexpr inline void traits(traits_t& t) const final{return src->traits(t); };

            constexpr inline static field_t _fields[] = {};

            uint16_t serialize(tree::builder& dst)const {
                //TODO: Possibly revise this one. For now it copies.
                return src->serialize(dst);
            }

            bool to_cpp(ostream& dst) const final{
                #if SDF_IS_HOST==true
                    dst<<std::format("Forward(node_{})",(void*)(&src));
                    return true;
                #else
                    return false;
                #endif
            }

            bool to_xml(xml& dst) const final{
                return false;
            }
        };

    }}

    namespace comptime_base {
        template <typename Attrs=default_attrs, template<typename, typename... Args> typename Src, typename... Args>
        constexpr inline impl_base::Forward<Attrs, Src, Args...> Forward ( const Src<Attrs,Args...>& ref ){
            return ref;
        }
    }
    namespace polymorphic_base {
        template <typename Attrs, template<typename, typename... Args> typename Src, typename... Args>
        constexpr inline impl_base::Forward<Attrs, Src, Args...> Forward ( const utils::dyn<Attrs,Src,Args...>& ref  ){
            return ref; 
        }
    }
    namespace comptime {
        template <typename Attrs=default_attrs, template<typename, typename... Args> typename Src, typename... Args>
        constexpr inline impl::Forward<Attrs, Src, Args...> Forward ( const Src<Attrs,Args...>& ref ){
            return ref;
        }
    }
    namespace polymorphic {
        template <typename Attrs, template<typename, typename... Args> typename Src, typename... Args>
        constexpr inline impl::Forward<Attrs, Src, Args...> Forward ( const utils::dyn<Attrs,Src,Args...>& ref  ){
            return ref; 
        }
    }
    namespace dynamic {
        template <typename Attrs>
        constexpr inline std::shared_ptr<utils::base_dyn<Attrs>>  Forward ( const std::shared_ptr<utils::base_dyn<Attrs>>& ref  ){
            return std::make_shared<impl::ForwardDynamic<Attrs>>(ref);
        }
    }               
}