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

#ifndef SDF_INTERNALS
#error Don't import manually, this can only be used internally by the library
#endif

#include "sdf.hpp"
#include <ostream>
#include <format>

namespace sdf{

namespace serialize{

    

bool fields2cpp (std::ostream& out, const void * node, fields_t fields, bool trailing_comma=true);
bool map2fields (const std::map<std::string,std::string>& map, const void * node, fields_t fields);

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
            using attrs_t = std::remove_reference_t<decltype(usdf)>::attrs_t::extras_t;
            //This works because by construction of the layout, the cfg of leaves is always at the beginning of the object.
            ((const attrs_t*)base)->to_cpp((sdf::ostream&)out);
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