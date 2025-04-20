#pragma once 

/**
 * @file serialize.hpp
 * @author karurochari
 * @brief Serialization for sdf.
 * @date 2025-04-19
 * It covers serialization & deserialization of attributes, and several helpers to implement code generation.
 * @copyright Copyright (c) 2025
 * 
 */

#include "sdf/commons.hpp"
#ifndef SDF_INTERNALS
#error Don't import manually, this can only be used internally by the library
#endif

#include "sdf.hpp"
#include <ostream>
#include <format>

namespace sdf{

namespace serialize{

    
/**
 * @brief Convert fields from an SDF node to cpp code.
 * 
 * @param out the destination ostream
 * @param node address of the node (sdf.addr())
 * @param fields list of fields (sdf.fields())
 * @param trailing_comma add one last comma to the list
 * @return true if no error was met
 * @return false errors were met
 */
bool fields2cpp (std::ostream& out, const void * node, fields_t fields, bool trailing_comma=true);

/**
 * @brief Apply an std::map into fields of an SDF node.
 * 
 * @param map 
 * @param node 
 * @param fields 
 * @return true 
 * @return false 
 */
bool map2fields (const std::map<std::string,std::string>& map, const void * node, fields_t fields);

template<sdf::attrs_i T>
bool attrs2cpp(const typename T::extras_t& attrs, std::ostream& out){
    return false;
}

template<> 
bool attrs2cpp<sdf::idx_attrs<>>(const sdf::idx_attrs<>::extras_t& attrs, std::ostream& out){
    out<<attrs.uid<<","<<attrs.gid<<","<<attrs.idx<<","<<attrs.weak;
    return true;
}

template<> 
bool attrs2cpp<sdf::color_attrs>(const sdf::color_attrs::extras_t& attrs, std::ostream& out){
    out<<attrs.r<<","<<attrs.g<<","<<attrs.b<<","<<attrs.a;
    return true;
}


//TODO: make it const and use the const version of tree_visit_pre once ready.
/**
 * @brief Compile an SDF into C++ code
 * 
 * @param usdf the referenced sdf
 * @param out the destination ostream
 * @return true 
 * @return false 
 */
bool sdf2cpp (sdf::sdf_i auto& usdf, std::ostream& out){
    struct entry_t{
        const char* tag;
        sdf::fields_t fields;
        void* base;
        size_t children;
        size_t current;
    };
    std::vector<entry_t> depth = {};

    static auto op = [&](const char* tag, sdf::fields_t fields, void* base, size_t children)->bool{
        //Handle comma separator for operator args
        if(!depth.empty()){
            auto& entry = depth.back();
            if(entry.current+1!=entry.children)out << ",";
        }

        //Leaf
        if(children==0){
            out<<tag<<"({";
            fields2cpp(out, base, fields);
            out << "{";
            using attrs_t = std::remove_reference_t<decltype(usdf)>::attrs_t;
            //This works because by construction of the layout, the cfg of leaves is always at the beginning of the object.
            attrs2cpp<attrs_t>((*(const typename attrs_t::extras_t*)base),out);
            out << "}}";
            out <<")";

            if(depth.empty())return true;
        }
        //Operator
        else{
            out<<tag<<"(";
            depth.push_back({tag,fields,base,children,children});
        }

        //Escape nesting
        while(!depth.empty()){
            auto& entry = depth.back();
            if(entry.current==0){
                out<<",{";
                fields2cpp(out, entry.base, entry.fields,false);
                out<<"})";
                depth.pop_back();
            }
            else{
                entry.current--;
                break;
            }
        }

        return true;
    };

    return usdf.tree_visit_pre(op);
};



}

}