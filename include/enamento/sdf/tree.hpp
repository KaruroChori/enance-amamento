#pragma once

#ifndef SDF_INTERNALS
#error "Don't import manually, this can only be used internally by the library"
#endif

#include "commons.hpp"
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
        //Primitives
        Zero = 0x1,
        Demo,
        Sphere,
        Box,
        Plane,

        //Special
        OctaSampled3D,

        //Modifiers
        Material,

        //Operators
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

    bool build(){
        return true;
    }

    bool make_shared(size_t idx) const{
        return global_shared.copy(idx,{bytes.data(),bytes.size()});
    }
};

}
}