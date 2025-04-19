#pragma once

//TODO: this will be removed and moved into sampler/texture***

#include "shared.hpp"
#include <string_view>
#include <glm/glm.hpp>

namespace texture2D_uncompressed{

    /**
     * @brief Prefix to describe a 2D buffer format with up to 4 layers.
     * 
     */
    struct format_t{
        enum class depth_t : uint8_t {D1,D2,D4,D8,D12,D16,D32,D64};
        enum class type_t  : uint8_t {NONE,INTEGER,FIXED,FLOATING};

        struct channel_t{
            depth_t depth :     3;
            uint8_t has_sign :  1;
            uint8_t type :      2;
            uint8_t extras :    2;    //For fixed notation, the proportion of bits used for the decimal part. 0 = 20%, 1 = 40%, 2 = 60%, 3 = 80%
        }channels[4];

        uint16_t width;
        uint16_t height;
    };

    //TODO: from here on it is all to implement anew
    struct body{
        uint32_t width: 16;
        uint32_t height: 16;
        uint32_t padding: 16;   //Esxtra bytes per row.
        uint32_t depth: 16;     //Only vec4 for now.
        uint8_t* data;
    };

    typedef size_t handle;

    #pragma omp declare target
    static shared index (sizeof(body)*512);
    #pragma omp end declare target


    handle load(std::string_view file);
    void unload(handle idx);

    enum wrap_t{
        NO_WRAP = 0,
        WRAP_X = 1,
        WRAP_Y = 2,
        WRAP_XY = 3,
    };

    enum filter_t{
        NEAREST,
        LINEAR,
        BICUBIC,
        MIXED
    };

    template<wrap_t wrap, filter_t filter>
    glm::vec4 texture2D(handle uid, glm::vec2 pos, float bias){
        auto pick = index[uid];
        if(pick.data==nullptr)return {};

        if constexpr(wrap==WRAP_X){pos.x=fract(pos.x);pos.y=clamp(pos.y,0.f,1.f);}
        else if constexpr(wrap==WRAP_Y){pos.y=fract(pos.y);pos.x=clamp(pos.x,0.f,1.f);}
        else if constexpr(wrap==WRAP_XY){pos=fract(pos);}
        else {pos.x=clamp(pos.x,0.f,1.f);pos.y=clamp(pos.y,0.f,1.f);}

        if constexpr(filter==NEAREST){
            uint8_t* address = (int(roundf(pos.x*pick.width))+int(roundf(pos.y*pick.height))*(pick.width+pick.padding))*pick.depth;
            glm::vec4* ret = (glm::vec4*) address;
            return *ret;
        }
        else if constexpr(filter==LINEAR){
            //Sample at floor and ceil for x and y, average them.
            return {};
        }
        else{
            //TODO not supported;
            return {};
        }
    }

}