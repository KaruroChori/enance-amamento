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

#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>
#include <memory>
#include <type_traits>

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


//#pragma omp declare target

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

}

#define SDF_INTERNALS
    #include "tree.hpp"
#undef SDF_INTERNALS

namespace sdf{

/**
 * @brief Interface to be a valid attribute type for the sdf.
 * 
 * @tparam T 
 */
template<typename T>
concept attrs_i = requires(const T& self, const T& self2, ostream& ostrm, xml& oxml){
    std::is_same<decltype(self.distance),float>();
    std::is_same<decltype(self.fields),typename T::extras_t>();
    {self+self2} -> std::convertible_to<typename T::extras_t> ;
    {self.fields.to_cpp(ostrm)} -> std::same_as<bool> ;
    {self.fields.to_xml(oxml)} -> std::same_as<bool> ;
    //{self.fields.from_xml(oxml)} -> std::same_as<bool> ;
};

/**
 * @brief Interface to be an SDF
 * 
 * @tparam T 
 */
template<typename T>
concept sdf_i  = attrs_i<typename T::attrs_t> && requires(
    const T self, glm::vec2 pos2d, glm::vec3 pos3d, 
    traits_t traits, ostream& ostrm, xml& oxml, 
    const path_t* paths, tree::builder& otree
){
    {self.operator()(pos3d)} -> std::same_as<typename T::attrs_t>;
    {self.sample(pos3d)} -> std::convertible_to<float>;
    {self.traits(traits)}-> std::same_as<void>;
    {self.name()} -> std::same_as<const char*>;
    {self.fields()}-> std::same_as<fields_t>;
    {self.fields(paths)} -> std::same_as<fields_t> ;
    {self.to_cpp(ostrm)} -> std::same_as<bool> ;
    {self.to_xml(oxml)} -> std::same_as<bool> ;
    {self.to_tree(otree)} -> std::same_as<uint64_t> ;
    //{self.from_xml(oxml)} -> std::same_as<bool> ;

    {self.is_visible()} -> std::same_as<visibility_t> ;
};

}