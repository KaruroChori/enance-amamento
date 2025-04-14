#pragma once

#include <cstdint>
#include <numbers>
#include <vector>
#include "sdf/commons.hpp"

namespace sampler{

namespace octatree3D{

using namespace glm;

struct node_base{  
    uint32_t children[2][2][2] = {};
};

template <sdf::attrs_i Attrs>
struct node: node_base{
    Attrs attrs;
};

struct header_t{
    uint reached_depth; 
    uint max_depth; 
    size_t cells; 
    float box_size; 
    glm::vec3 offset;
};

template <sdf::sdf_i SDF>
struct builder{
    private:
        uint        max_steps = 10;
        uint        reached_depth = 0;
        uint        depth = 0;

        const SDF&  sdf;
        float       box_size;
        glm::vec3   offset;
        
        std::vector<node<typename SDF::attrs_t>> data;

    public:

        /**
         * @brief Resets the current builder structure to recompute from zero.
         */
        inline void reset(){
            reached_depth = 0;
            depth = 0;
            data.clear();
        }

        inline bool build(){
            reset();
            auto ret =  generate(box_size);
            data[0]=data.back();
            return true;
        }

        bool make_shared(size_t idx) const{
            auto ret = global_shared.reserve(idx, (data.size()-1)*sizeof(sampler::octatree3D::node<typename SDF::attrs_t>)+sizeof(header_t));
            if(ret==false)return false;
            auto slot = global_shared[idx];
            header_t *head = (header_t *)slot.base;
            (*head)=stats();
            memcpy((void*)((header_t*)slot.base+1),(const void*)data.data(),(data.size()-1)*sizeof(sampler::octatree3D::node<typename SDF::attrs_t>));
            global_shared.sync(idx);
            return true;
        }
    
        inline auto stats() const{
            return header_t{reached_depth, max_steps, data.size()-1,box_size,offset};
        }

        builder(const SDF& sdf, uint max_steps, const glm::vec3& offset, float box_size):max_steps(max_steps),sdf(sdf),offset(offset),box_size(box_size){}

        builder(const SDF& sdf, uint max_steps):max_steps(max_steps),sdf(sdf){
            sdf::traits_t traits;
            sdf.traits(traits);
            offset=(traits.outer_box.min+traits.outer_box.max)/2.0f;
            auto delta = abs(traits.outer_box.min-traits.outer_box.max);
            box_size=max(delta.x,max(delta.y,delta.z))*2.0f; //TODO: Move back the factor to a more reasonable minimum once rendering issues are sorted out.
        }

        ~builder(){}

    private:

        uint32_t generate( float box_size, vec3 boxcenter={0,0,0}, uint depth =0){
            //TODO: the main issue is that we should take the leaf elements and from there casting rays in all direction to look for a surface.
            //Then, and only then, we have a sampled value for normals and attributes. Otherwise for coarser approximations the calculated attributes makes no sense.
            typename SDF::attrs_t computed =  sdf(boxcenter+offset);
            size_t t = 0;
            if(abs(computed.distance)>box_size*std::numbers::sqrt3 || depth>max_steps){
                sampler::octatree3D::node<typename SDF::attrs_t>* node;
                    {
                    //std::lock_guard<std::mutex>grd(mtx);
                    reached_depth=max(reached_depth,depth);
                    data.push_back({});
                    node = &data.back();
                    t = data.size()-1;
                    }
                    node->attrs=computed;
                    memset(node->children,0,8*4);

                    return t;
            } else{
                uint32_t split_computed[2][2][2];
                auto halfbox = box_size/2.0f;
                {
                    //#pragma omp  parallel for collapse(3) schedule(static,1) 
                    for(int x=0;x<2;x++){
                        for(int y=0;y<2;y++){
                            for(int z=0;z<2;z++){
                                split_computed[x][y][z] = generate( halfbox, boxcenter+box_size*vec3{(x-0.5f),(y-0.5f),(z-0.5f)},depth+1);

                            }
                        }
                    }
                }

                sampler::octatree3D::node<typename SDF::attrs_t>* node;
                {
                    //std::lock_guard<std::mutex>grd(mtx);
                    reached_depth=max(reached_depth,depth);
                    data.push_back({});
                    node = &data.back();
                    t = data.size()-1;
                    }
                    node->attrs=computed;
                    memcpy(node->children,split_computed,8*4);

                    return t;
            }
        }

};

}

}