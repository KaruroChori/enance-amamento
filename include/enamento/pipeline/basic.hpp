#pragma once

/**
 * @file basic.hpp
 * @author karurochari
 * @brief Pipelines, for rendering and common compute tasks on SDF!
 * @date 2025-03-25
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <glm/glm.hpp>
#include <utility>
#include "solver/projection/base.hpp"
#include "../sdf/sdf.hpp"

namespace pipeline{

namespace utils{
    std::pair<uint32_t,int> pow2ceil(uint32_t n){
        if (n == 0) return {1,0};

        // Check if n is already a power of two
        if ((n & (n - 1)) == 0)
            return {n,32 - __builtin_clz(n)};
    
        // Calculate number of bits needed to represent n
        int bits = 32 - __builtin_clz(n);
        return {1u << bits,bits};
    }

    /**
     * @brief Look for the closest fitting power of two 2d vector fitting screen
     * 
     * @param screen the size of the window to be fit
     * @return std::pair<glm::ivec2,int> the resulting window and the maximum level of subdivisions which can be reached.
     */
    std::pair<glm::ivec2,int> fitting2power(const glm::ivec2& screen){
        auto x = pow2ceil(screen.x);
        auto y = pow2ceil(screen.y);
        return {{x.first,y.first},std::min(x.second,y.second)};
    }
}

struct material_t{
    typedef int res_ref;
    
    struct albedo_t{
        enum{COLOR, TRIPLANAR, STYLEBLIT} type;
        struct color_t{
            glm::vec3    rgb;
            float   transmission;
        };
        union{
            color_t     color;
            res_ref     triplanar;
            res_ref     stylebit;
        };
    }albedo;
};


template<typename SDF, bool cone_march = false>
struct demo{
    private:
        int device;

        int display_width=0, display_height=0;
        int render_width=0, render_height=0;
        int allocated=0;
        float scale = 1.0;

        typedef typename solver::projection::base<SDF>::output_t fields_t;

        //On device
        fields_t*   layer_0 = nullptr;
        glm::vec4*  sobel_base = nullptr;
        glm::vec4*  sobel_dilate = nullptr;
        material_t* materials = nullptr;

        //On host
        glm::u8vec4*output = nullptr;
        
        solver::projection::base<SDF> scene;

    public:

    void cleanup(){
        omp_target_free(layer_0,device);
        omp_target_free(sobel_base,device);
        omp_target_free(sobel_dilate,device);
        omp_free(output);
    }

    demo(int device, SDF& sdf, const material_t* mats, size_t mats_n):device(device),scene(sdf){
        materials = (material_t*) omp_target_alloc(sizeof(material_t)*mats_n,device);
        omp_target_memcpy(materials,mats,mats_n*sizeof(material_t),0,0,device,omp_get_device_num());
    }
    
    ~demo(){
        cleanup();
        omp_target_free(materials,device);
    }


    int resize(uint _display_width, uint _display_height, float _scale){
        //TODO: check for alloc failure
        if(display_height*display_width>=_display_height*_display_width && allocated>=(int)(_display_height*_display_width/_scale/_scale)){
            display_width=_display_width;display_height=_display_height;scale=_scale;

            return 0;
        }

        display_width=_display_width;display_height=_display_height;scale=_scale;
        allocated = display_width*display_width/scale/scale;
        
        cleanup();

        layer_0 = (fields_t*) omp_target_alloc((display_width*display_width/scale/scale)*sizeof(fields_t),device);
        sobel_base = (glm::vec4*) omp_target_alloc((display_width*display_width)*sizeof(glm::vec4),device);
        sobel_dilate = (glm::vec4*) omp_target_alloc((display_width*display_width)*sizeof(glm::vec4),device);
        output = (glm::u8vec4*) omp_alloc(display_width*display_height*sizeof(glm::u8vec4));

        return 0;
    }

    void set_camera(const solver::projection::screen_camera_t& camera){
        resize(camera.canvas_width, camera.canvas_height, camera.resolution_scale);

        render_height = display_height/scale;
        render_width = display_width/scale;

        scene.camera = camera;
    }

    glm::vec3 raycast(const glm::vec2& point){
        using namespace glm;
        vec2 coo = (point*scale-0.5f*vec2{display_width,display_height})/(float)display_height;
        vec3 ret;
        #pragma omp target device(device)
        {
            ret = scene.raycast(coo);
        }
        return ret;
    }

    glm::u8vec4* render(glm::u8vec4* out=nullptr){
        using namespace glm;

        if(out==nullptr)out=this->output;

        //First Pass (only one layer in this rendering pipeline)
        #pragma omp target teams device(device) is_device_ptr(layer_0) is_device_ptr(sobel_base) is_device_ptr(sobel_dilate)
        {

            #pragma omp distribute parallel for collapse(2) schedule(static,1)
            for (int i = 0; i < render_height; i++) {
                for (int j = 0; j < render_width; j++) {
                    vec2 coo = (vec2{j,i}*scale-0.5f*vec2{display_width,display_height})/(float)display_height;
                    layer_0[i*render_width+j]= scene.render(coo);
                }
            }
        }

        //Edge detection
        #pragma omp target teams device(device) is_device_ptr(layer_0) is_device_ptr(sobel_base) is_device_ptr(sobel_dilate)
        {             
            #pragma omp distribute parallel for collapse(2) schedule(static,1)
            for (int i = 0; i < display_height; i++) {
                for (int j = 0; j < display_width; j++) {

                    static auto sample = [&](ivec2 pos) {
                        pos=clamp(vec2(pos)/scale,{0,0},{display_width-1,display_height-1});
                        return vec4(layer_0[pos.y*render_width+pos.x].idx,layer_0[pos.y*render_width+pos.x].gid,layer_0[pos.y*render_width+pos.x].uid,layer_0[pos.y*render_width+pos.x].depth);
                    };

                    vec4 n[9];
                    {
                        ivec2 coord = {j,i};
                        n[0] = sample(coord + ivec2(  -1, -1));
                        n[1] = sample(coord + ivec2(  0, -1));
                        n[2] = sample(coord + ivec2(  1, -1));
                        n[3] = sample(coord + ivec2( -1, 0));
                        n[4] = sample(coord);
                        n[5] = sample(coord + ivec2(  1, 0));
                        n[6] = sample(coord + ivec2( -1, 1));
                        n[7] = sample(coord + ivec2(  0, 1));
                        n[8] = sample(coord + ivec2(  1, 1));
                    }

                    vec4 sobel_edge_h = n[2] + (2.0f*n[5]) + n[8] - (n[0] + (2.0f*n[3]) + n[6]);
                    vec4 sobel_edge_v = n[0] + (2.0f*n[1]) + n[2] - (n[6] + (2.0f*n[7]) + n[8]);
                    vec4 sobel = sqrt((sobel_edge_h * sobel_edge_h) + (sobel_edge_v * sobel_edge_v));

                    auto ret = 1.0f - clamp(sobel,{0,0,0,0},{1,1,1,0.3});
                    //Baaaaasically, I use the color information (later material idex and add a contribution for the distance which fades with distance itself(to avoid horizon artifacts) )
                    sobel_base[i*display_width+j]={((ret.x+ret.y+ret.z)/3.0-sobel.w/n[4].w)>0.5?1.0:0.0,(sobel.x+sobel.y+sobel.z)/3.0,ret.w,sobel.w};
                }
            }
        }

        //Dilate
        #pragma omp target teams device(device) is_device_ptr(layer_0) is_device_ptr(sobel_base) is_device_ptr(sobel_dilate)
        {             
            #pragma omp distribute parallel for collapse(2) schedule(static,1) 
            for (int i = 0; i < display_height; i++) {
                for (int j = 0; j < display_width; j++) {
                    static auto fill = [&](ivec2 pos, int radius) {
                        pos=clamp(pos,{0,0},{display_width-1,display_height-1});
                        float acc = 1.0;
                        float w = 0.0;
                        for(int i = -radius; i <= radius; ++i) {
                            for(int j = -radius; j <= radius; ++j) {
                                float kernel = (i*i+j*j<radius*radius)?1.0:0.0;// texture(iChannel1, vec2(0.5f) + kuv).x;
                                ivec2 newpos = pos+ivec2{j,i};
                                newpos=clamp(newpos,{0,0},{display_width-1,display_height-1});

                                vec4 tex = sobel_base[newpos.x+newpos.y*display_width];
                                vec4 v = tex - vec4{kernel};
                                if(v.x < acc) {
                                    acc = v.x;
                                    w = kernel;
                                }
                            /* if(v.y < acc.y) {
                                    acc.y = v.y;
                                    w.y = kernel;
                                }
                                if(v.z < acc.z) {
                                    acc.z = v.z;
                                    w.z = kernel;
                                }
                                if(v.w < acc.w) {
                                    acc.w = v.w;
                                    w.w = kernel;
                                }*/
                            }
                        }
                        return acc + w;
                    };
                    
                    ivec2 pos=clamp(vec2({j,i})/scale,{0,0},{display_width-1,display_height-1});
                    auto pixel = layer_0[pos.y*render_width+pos.x];

                    //TODO scale down based on the sobol of the distance field.
                    auto t = fill({j,i},5.0/clamp(pixel.depth/5.0f,1.f,5.f)); 
                    sobel_dilate[i*display_width+j]=vec4{t, sobel_base[i*display_width+j].y,sobel_base[i*display_width+j].z,sobel_base[i*display_width+j].w};
                }
            }
        }

        //u8vec4 *tmp = output;   //For some reason using output directly is illegal. I need a local copy. No idea why.
        #pragma omp target data map(from: out[0:display_width*display_height]) device(device)
        {
            #pragma omp target teams device(device) is_device_ptr(layer_0) is_device_ptr(sobel_base) is_device_ptr(sobel_dilate)
            {             
                #pragma omp distribute parallel for collapse(2) schedule(static,1)
                for (int i = 0; i < display_height; i++) {
                    for (int j = 0; j < display_width; j++) {
                        ivec2 pos=clamp(vec2({j,i})/scale,{0,0},{display_width-1,display_height-1});
                        auto pixel = layer_0[pos.y*render_width+pos.x];

                        auto& material = materials[pixel.idx];
                        if(material.albedo.type==material_t::albedo_t::COLOR){
                            out[i*display_width+j]=vec4{1.0,material.albedo.color.rgb.bgr()*sobel_dilate[i*display_width+j].z}*255.0f;
                        }
                    }
                } 
            }
        }
        //memcpy(out,output,display_width*display_height*sizeof(glm::u8vec4));
        return out;
    }
};

}