/**
 * @file comptime-render.cpp
 * @author karurochari
 * @brief A cute demo to show sdf rendering which is done at compile time
 * @date 2025-04-15
 * 
 * @copyright Copyright (c) 2025
 * 
 */

//NOTICE: Not working, not compiling. The reason is that clang does not support constexpr sqrt yet. Running this on GCC might work.

#include <glm/glm.hpp>

#define SDF_HEADLESS true
#include "sdf/sdf.hpp"

constexpr std::size_t ROWS = 640;
constexpr std::size_t COLS = 640;

constexpr std::array<std::array<sdf::default_attrs, COLS>, ROWS> render(const auto& sdf){
    std::array<std::array<sdf::default_attrs, COLS>, ROWS> images;
    for(uint i = 0; i < ROWS; i++){
        for(uint j = 0; j < COLS; j++){
            images[i][j]=sdf({i,j,0.0f});
        }
    }
    return images;
}

//TODO: this fails because the math library is not constexpr in c++23 & clang-21 :(
/*constexpr*/ auto fn = sdf::comptime::Sphere_t<sdf::default_attrs>(5);
/*constexpr*/ auto image = render(fn);

int main(){
    for(uint i = 0; i < ROWS/16; i++){
        for(uint j = 0; j < COLS/16; j++){
            printf("%c",(int)(image[i*16][j*16].distance)%128+32);
        }
        printf("\n");
    }
    return 0;
}