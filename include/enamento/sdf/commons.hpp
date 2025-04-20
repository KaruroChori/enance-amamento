#pragma once

/**
 * @file commons.hpp
 * @author karurochari
 * @brief Shared bits of SDF to expose some of its structures and interfaces without the full library
 * @date 2025-04-12
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <cmath>
#include <concepts>
#include <cstdint>
#include <memory>
#include <functional>
#include <type_traits>

#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>

#include "utils/tribool.hpp"

#ifdef SDF_HEADLESS
    #undef SDF_IS_HOST
    #define SDF_IS_HOST false

    #include "utils/shared.hpp"
    static shared_map<0> global_shared;
#else
    #ifndef SDF_SHARED_SLOTS
        #error Shared slots must be specified if the application is not used in headless mode.
    #endif
#endif

#if SDF_IS_HOST==true
    #include <pugixml.hpp>        //TODO: check headers and libs linked
    #include <ostream>
    namespace pugi{
        class xml_node;
    }
#else
    
#endif


////Helper structs & Consts////

namespace sdf{
//Useful constants to keep rendering consistent.
#pragma omp declare target
constexpr static float EPS = 2e-5;
constexpr static float MIX_EPS = 400e-2;
#pragma omp end declare target

#if SDF_IS_HOST==true
    using ostream = std::ostream ;
    using xml = pugi::xml_node ;
#else
    struct ostream{};
    struct xml{};
#endif



enum path_t{
    END /*Null terminated*/, LEFT, RIGHT
};

/**
* @brief Bounding box SDF
*/
struct bbox_t{
    glm::vec3 min = {-INFINITY,-INFINITY,-INFINITY};
    glm::vec3 max = {INFINITY,INFINITY,INFINITY};
};

struct traits_t{
    //TODO: replace with tribool array
    glm::ivec3  is_sym;                             //It has symmetries along the main axis
    tribool     is_exact_inner;                     //The SDF is exact in the outer region
    tribool     is_exact_outer;                     //The SDF is exact in the inner region
    tribool     is_bounded_inner;                   //The sampled value in the outer region is a lower bound for the real distance
    tribool     is_bounded_outer;                   //The sampled value in the inner region is a lower bound for the real distance
    tribool     is_rigid=tribool::unknown;          //The operation has a limited region of influence and preserves surfaces outside that.
    tribool     is_local=tribool::unknown;          //The operation can be locally disruptive, but at a distance the exactness is preserved.
    tribool     is_associative=tribool::unknown;    //The operation is associative with itself.
    bbox_t      outer_box;                          //Bounding box
};

typedef size_t shared_buffer;

/**
* @brief To capture fields on an SDF structure
* Used to construct the `fields` field.
*/
struct field_t{
    ///If readonly, changes will not be allowed from UI
    bool readonly = true;
    ///Types supported, a hardcoded small set are considered. Anything else `type_unknown`
    enum type_t{
        type_unknown,
        type_cfg, //Special one for the base cfg. It differs based on Attrs
        type_float, type_vec2, type_vec3,
        type_int, type_ivec2, type_ivec3, 
        type_bool, //Yes No
        type_tribool, //Yes No Unknown
        type_enum, //The map of allowed values and descriptions/mnemonics is encoded in desc.
        type_shared_buffer,
    }type = type_unknown;
    ///Support specialization for the widget type used for rendering. If not compatible, the default one is used instead.
    enum widget_t{
        widget_deftype,
        widget_color,
        widget_knob,
        //etc...
    }widget = widget_deftype;
    const char* name = nullptr;
    const char* desc = nullptr;
    size_t offset = 0;
    size_t length = 0;
    void* min = nullptr;
    void* max = nullptr;
    void* defval = nullptr;
    ///A function accepting a pointer to the value to test. Returns true if valid, false else. 
    bool(*validate)(const void* value) = nullptr;
};

struct fields_t{
    const field_t* data;
    size_t items;

    inline const field_t* begin(){return data;}
    inline const field_t* end(){return data+items;}
};

enum class visibility_t{
    HIDDEN,     //Hide the current node (and subtree)
    VISIBLE,    //Display it as usual
    SKIP,       //Ignore the current node, but render children (as union)
};

typedef std::function<bool(const char* name, fields_t fields, void* base, size_t children)> visitor_t;
typedef std::function<bool(const char* name, fields_t fields, const void* base, size_t children)> cvisitor_t;

}

////Tree Primitives////

#define SDF_INTERNALS
    #include "tree.hpp"
#undef SDF_INTERNALS

////Interfaces////

namespace sdf{

/**
 * @brief Interface to be a valid attribute type for the sdf.
 * 
 * @tparam T 
 */
template<typename T>
concept attrs_i = requires(const T& self, const T& self2, xml& oxml, const xml& ixml, T::extras_t& defattrs){
    std::is_same<decltype(self.distance),float>();
    std::is_same<decltype(self.fields),typename T::extras_t>();
    {self+self2} -> std::convertible_to<typename T::extras_t> ;

    //TODO: BLOCK TO DEPRECATE
    {self.fields.to_xml(oxml)} -> std::same_as<bool> ;
    //{self.fields.from_xml(ixml, defattrs)} -> std::same_as<bool> ;
};

/**
 * @brief Interface to be an SDF
 * 
 * @tparam T 
 */
template<typename T>
concept sdf_i  = attrs_i<typename T::attrs_t> && requires(
    const T self, T mutself,
    glm::vec2 pos2d, glm::vec3 pos3d, 
    traits_t traits, xml& oxml, 
    const path_t* paths, tree::builder& otree,
    const visitor_t& visitor, const cvisitor_t& cvisitor
){
    {self.operator()(pos3d)} -> std::same_as<typename T::attrs_t>;
    {self.sample(pos3d)} -> std::convertible_to<float>;
    
    {self.name()} -> std::same_as<const char*>;
    {self.fields()}-> std::same_as<fields_t>;
    {self.fields(paths)} -> std::same_as<fields_t> ;
    {self.addr()} -> std::same_as<const void*>;
    {mutself.addr()} -> std::same_as<void*>;
    {self.children()} -> std::same_as<size_t> ;
    {self.is_visible()} -> std::same_as<visibility_t> ;
    {self.traits(traits)}-> std::same_as<void>;

    {mutself.tree_visit_pre(visitor)} -> std::same_as<bool>;
    {mutself.tree_visit_post(visitor)} -> std::same_as<bool>;
    {self.ctree_visit_pre(cvisitor)} -> std::same_as<bool>;
    {self.ctree_visit_post(cvisitor)} -> std::same_as<bool>;

    //TODO: BLOCK TO DEPRECATE
    {self.to_xml(oxml)} -> std::same_as<bool> ;
    {self.to_tree(otree)} -> std::same_as<uint64_t> ;
    //{self.from_xml(oxml)} -> std::same_as<bool> ;

};

}

////Attribute structures////

namespace sdf{

    /**
     * @brief Most basic implementation of attributes for SDF, including only the bare minimum.
     */
    struct basic_attrs{
        /**
         * @brief Additional fields, empty on a default implementation
         */
        struct extras_t{
            bool to_xml(xml& n) const{
                #if SDF_IS_HOST==true
                    return false; 
                #else
                return false;
                #endif
            }
        };
    
        constexpr static inline const extras_t SKY={};
    
        float distance;
        glm::vec3 normals;
    
        [[no_unique_address]] extras_t fields = {};
    
        friend inline extras_t operator+(const basic_attrs& l, const basic_attrs& r){
            return (glm::abs(l.distance)<glm::abs(r.distance))?l.fields : r.fields;
        }
    };
    
    /**
     * @brief Alernative implementation packing enough information to have a full rendering pipeline.
     * It provides a material index, identifiers and a flag to control blending.
     * @tparam BLEND If true, materials outside the boundaries of the original shapes will be blended via dithering. Else, hard edge.
     */
    template<bool BLEND = true>
    struct idx_attrs{
        struct extras_t{
            uint32_t uid: 12 = 0;       ///Object identity. Zero for "not assigned", basically one which is not being tracked.
            uint32_t gid:  9 = 0;       ///Object group. Zero for default group, 511 for sky.
            uint32_t idx: 10 = 0;       ///Material index, zero for special NONE, preventing its rendering.   
            uint32_t weak: 1 = true;    ///If true, don't use this material for contributions in operators (unless the other is also weak).
        
            bool to_xml(xml& n) const{
                #if SDF_IS_HOST==true
                    n.append_attribute("uid").set_value(this->uid);
                    n.append_attribute("gid").set_value(this->gid);
                    n.append_attribute("idx").set_value(this->idx);
                    n.append_attribute("weak").set_value((bool)this->weak);
                    return true; 
                #else
                return false;
                #endif
            }
            bool from_xml(const xml& n, extras_t& defval){
                #if SDF_IS_HOST==true
                    //TODO: Add checks
                    if(strcmp(n.attribute("uid").as_string(""),"*")==0)this->uid=++defval.uid;
                    else this->uid = n.attribute("uid").as_uint(0);
                    this->gid = n.attribute("gid").as_uint(defval.gid);
                    this->idx = n.attribute("idx").as_uint(defval.idx);
                    this->weak = n.attribute("weak").as_bool(defval.weak);
                    return true;
                #else
                    return false;
                #endif
            }
        };
        
        consteval static extras_t SKY(){return extras_t{0,511,0,true};}
    
        float distance;
        glm::vec3 normals;
    
        [[no_unique_address]] extras_t fields = {};
    
        //TODO: Probably move it to its special utility namespace.
        constexpr static inline float rand(const glm::vec2& co){
            auto t = sinf(dot(co, glm::vec2(12.9898f, 78.233f))) * 43758.5453f;
            //Forced to use this in place of glm::frac as floor is not available on the offloaded target for some strange and absurd reason.
            return glm::fract(t);
        }
    
        friend inline extras_t operator+(const idx_attrs& l, const idx_attrs& r){
            if(l.fields.weak && !r.fields.weak){return r.fields;}
            if(r.fields.weak && !l.fields.weak){return l.fields;}
            else if constexpr(BLEND){
                    auto labs =glm::abs(l.distance), rabs=glm::abs(r.distance);
    
                    float pick = rand({l.distance,r.distance});
                    auto lw = rabs/(rabs+labs);
                    //Clamping removes flickering due to numerical precision.
                    if( lw>0.99 || (lw>pick && lw>0.01))return l.fields;
                    else return r.fields;
            }
            else return (glm::abs(l.distance)<glm::abs(r.distance))?l.fields : r.fields;
        }
    };
    
    
    /**
     * @brief Alernative basic implementation providing spectral emission data and tranmission value.
     * 
     */
    struct color_attrs{
        struct extras_t{
            uint8_t r;
            uint8_t g;
            uint8_t b;
            uint8_t a;
    
            bool to_xml(xml& n) const{
                #if SDF_IS_HOST==true
                    return false; 
                #else
                    return false;
                #endif
            }
        };
    
        consteval static extras_t SKY(){return extras_t{44,30,212,255};}
        
        float distance;
        glm::vec3 normals;
    
        [[no_unique_address]] extras_t fields = {};
    
        friend inline extras_t operator+(const color_attrs& l, const color_attrs& r){
            if(l.fields.a == 0){return r.fields;}
            if(r.fields.b == 0){return l.fields;}
            else return (glm::abs(l.distance)<glm::abs(r.distance))?l.fields : r.fields;
        }
    };
    
}