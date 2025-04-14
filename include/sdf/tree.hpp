#pragma once

#ifndef SDF_INTERNALS
#error Don't import manually, this can only be used internally by the library
#endif

#include "sdf/sdf.hpp"
#include "utils/shared.hpp"
#include <cstdint>
#include <cstring>
#include <map>
#include <vector>

namespace sdf{

namespace utils{
    template<typename Attrs>
    struct tree_idx;
}

namespace tree{

namespace op_t {
    enum type_t : uint16_t{
        //PRIMITIVES
        Zero = 0x1,
        Demo,
        OctaSampled3D,
        Sphere,
        Box,
        Plane,

        //Modifiers
        Material,

        //OPERATORS
        Join = 128,
        Cut,
        Common,
        Xor,

        Rotate,
        Scale,
        Translate,
        
        SmoothJoin,
    };

    enum mod_t : uint16_t{
        Located = 0x100,
        Optional = 0x200,
        LOD = 0x400,
    };
};

struct instance{

    struct named_t{
        uint16_t gid;
        uint16_t uid;
        uint16_t idx;
    };
    
    shared bytes;
    shared map;


    /*uint8_t*    bytes = nullptr;
    size_t      bytes_size = 0;
    named_t*    map = nullptr;    //Index to quickly find an object
    size_t      map_size = 0; */

    instance(const std::vector<uint8_t>& nbytes, const  std::map<std::pair<uint16_t,uint16_t>,uint16_t>& nmap):bytes(nbytes.size()),map(nmap.size()*sizeof(instance::named_t)){
            bytes.provide((void*)nbytes.data());
            int j=0;
    }
    
    ~instance(){}

};

struct builder{
    std::vector<uint8_t> bytes = {0,0,0,0,0,0};
    std::map<std::pair<uint16_t,uint16_t>,uint16_t> named_refs;
    uint64_t offset = 8;    //I must be 8 to avoid alignment issues :/. In general I must be **VERY** careful of alignment when packing this data structure.

    uint64_t push(op_t::type_t opcode, const uint8_t* data, size_t len){
        //printf("\n%d %d - %zu : %d\n", offset%8, offset, len, opcode);

        //First hword is the parent address, followed by bytes of data. Return the address of the first byte of data (skip parent address)
        //bytes.resize(offset+len+2+2);
        //bytes.insert(bytes.end(),(const uint8_t*)&opcode,(const uint8_t*)&opcode+2);
        
        bytes.push_back((int)opcode&0xff);
        bytes.push_back(((int)opcode>>8)&0xff);
        bytes.insert(bytes.end(),data,data+len);
        for(uint i = 0;i<len%8+8-2;i++)bytes.push_back({0xac});
        auto ret = offset;
        offset=bytes.size()+2;
        return ret;
    }

    void close(uint32_t root){
        //Write the offset for the first node in the first position.
        memcpy(bytes.data(),&root,4);
    }

    uint64_t next(){
        return offset;
    }

    /*instance build(){
        return {bytes, named_refs};
    }*/

    bool build(){
        return true;
    }

    bool make_shared(size_t idx) const{
        return global_shared.copy(idx,{bytes.data(),bytes.size()});
    }
};

}
}