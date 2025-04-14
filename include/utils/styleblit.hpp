#pragma once

/**
 * @file styleblit.hpp
 * @author karurochari
 * @brief Implementation of styleblit to apply normal-based material mapping onto SDFs
 * Derived from https://github.com/jamriska/styleblit/blob/master/styleblit/styleblit_main.frag
 * @date 2025-03-12
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <glm/glm.hpp>

namespace styleblit{

using namespace glm;

struct main{
    sampler2D target;
    sampler2D source;
    sampler2D noise;

    vec2 targetSize;
    vec2 sourceSize;
    float threshold;

    static float sum(vec3 xyz) { return xyz.x + xyz.y + xyz.z; }

    static float frac(float x) { return x-floor(x); }

    static vec4 pack(vec2 xy) {
        float x = xy.x/255.0;
        float y = xy.y/255.0;
        return vec4(frac(x),floor(x)/255.0,
                    frac(y),floor(y)/255.0);
    }

    static bool inside(vec2 uv,vec2 size){
        return (all(greaterThanEqual(uv,vec2(0,0))) && all(lessThan(uv,size)));
    }

    vec2 RandomJitterTable(vec2 uv){
        return texture2D(noise,(uv+vec2(0.5,0.5))/vec2(256,256)).xy;
    }

    vec2 SeedPoint(vec2 p,float h){
        vec2 b = floor(p/h);
        vec2 j = RandomJitterTable(b);  
        return floor(h*(b+j));
    }

    vec2 NearestSeed(vec2 p,float h){
        vec2 s_nearest = vec2(0,0);
        float d_nearest = 10000.0;

        for(int x=-1;x<=+1;x++){
            for(int y=-1;y<=+1;y++){
                vec2 s = SeedPoint(p+h*vec2(x,y),h);
                float d = length(s-p);
                if (d<d_nearest){
                    s_nearest = s;
                    d_nearest = d;     
                }
            }
        }

        return s_nearest;
    }

    vec3 GS(vec2 uv) { return texture2D(source,(uv+vec2(0.5f,0.5f))/sourceSize).rgb; }
    vec3 GT(vec2 uv) { return texture2D(target,(uv+vec2(0.5f,0.5f))/targetSize).rgb; }

    vec2 ArgMinLookup(vec3 targetNormal){
        return vec2(targetNormal.x,targetNormal.y)*sourceSize;
    }

    vec4 operator()(const vec2& xy){
        vec2 p = xy-vec2(0.5f,0.5f);
        vec2 o = ArgMinLookup(GT(p));

        for(int level=6;level>=0;level--)
        {
            vec2 q = NearestSeed(p,pow(2.0f,float(level)));
            vec2 u = ArgMinLookup(GT(q));
            
            float e = sum(abs(GT(p)-GS(u+(p-q))))*255.0f;
            
            if (e<threshold){
                o = u+(p-q); if (inside(o,sourceSize)) { break; }
            }
        }

        return pack(o);
    }
    
};


// Derived from https://github.com/jamriska/styleblit/blob/master/styleblit/styleblit_blend.frag

struct blend{
    sampler2D NNF;
    sampler2D sourceStyle;
    sampler2D targetMask;
    vec2 targetSize;
    vec2 sourceSize;

    constexpr static inline int BLEND_RADIUS = 1;

    vec2 unpack(const vec4& rgba){
        return vec2(rgba.r*255.0f+rgba.g*255.0f*255.0f,
                    rgba.b*255.0f+rgba.a*255.0f*255.0f);
    }

    vec4 main(const vec2& xy){ 
        vec4 sumColor = vec4(0.0f,0.0f,0.0f,0.0f);
        float sumWeight = 0.0f;
        
        if (texture2D(targetMask,(xy)/targetSize).a>0.0f){
            for(int oy=-BLEND_RADIUS;oy<=+BLEND_RADIUS;oy++){
                for(int ox=-BLEND_RADIUS;ox<=+BLEND_RADIUS;ox++){
                    if (texture2D(targetMask,(xy+vec2(ox,oy))/targetSize).a>0.0){
                        sumColor += texture2D(sourceStyle,((unpack(texture2D(NNF,(xy+vec2(ox,oy))/targetSize))-vec2(ox,oy))+vec2(0.5f,0.5f))/sourceSize);
                        sumWeight += 1.0f;
                    }
                }
            }
        }
        
        return (sumWeight>0.0) ? sumColor/sumWeight : texture2D(sourceStyle,vec2(0.0f,0.0f));
    }
};

void styleblit(int    targetWidth,
               int    targetHeight,
               GLuint texTargetNormals,
               int    sourceWidth,
               int    sourceHeight,
               GLuint texSourceNormals,
               GLuint texSourceStyle,
               float  threshold,
               int    blendRadius,
               bool   jitter);


struct noise_t{
    uint width, height;
    uint8_t* buffer = nullptr;

    noise_t(uint width, uint height):width(width),height(height){
        buffer = new unsigned char[width*width*4]; 
        for(int i=0;i<width*width*4;i++) { buffer[i] = (float(rand())/float(RAND_MAX))*255.0f; }
    }

    ~noise_t(){delete [] buffer;}

    vec4 operator()(const vec2& pos ){
        uint x = pos.x * width, y = pos.y*height;
        return {buffer[x*4+4*y*width],buffer[x*4+4*y*width+1],buffer[x*4+4*y*width+2],buffer[x*4+4*y*width+3]};
    }
};


}