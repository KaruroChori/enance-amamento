#pragma once

/**
 * @file octa-sampled.hpp
 * @author karurochari
 * @brief SDF which is sampled from an octatree representation
 * @date 2025-03-17
 * 
 * @copyright Copyright (c) 2025
 * 
 */


#ifndef SDF_INTERNALS
#error Don't import manually, this can only be used internally by the library
#endif

#include "sampler/octtree-3d.hpp"
#include "utils/tribool.hpp"
#include <cmath>
#include <cstddef>
#include <numbers>

#include "../sdf.hpp"
#include "../tree.hpp"


namespace sdf{

    namespace configs{
    }

    namespace{namespace impl_base{

        template <typename Attrs=default_attrs>
        struct OctaSampled3D{
            using attrs_t = Attrs;
            [[no_unique_address]] Attrs::extras_t cfg;

            typedef size_t handle_t;
            handle_t _handle = 0;

            size_t depth;

            vec3 offset;
            float size;

            inline sampler::octatree3D::node<Attrs>* handle() const{
                return ( sampler::octatree3D::node<Attrs>*)(((sampler::octatree3D::header_t*)global_shared[_handle].base)+1);
            }


            struct search_t{
                uint32_t idx;
                float size;
                vec3 center;
                size_t depth;
            };

            constexpr inline search_t search(const glm::vec3& pos) const{
                const sampler::octatree3D::node<Attrs>* current = handle();
                uint32_t current_idx=0;
                vec3 current_pos = {0,0,0};
                float current_size = size;
                size_t depth=0;
                while(true){
                    bvec3 coo = {pos.x>current_pos.x,pos.y>current_pos.y,pos.z>current_pos.z};
                    uint32_t child_idx = current->children[coo.x][coo.y][coo.z];
                    if(child_idx!=0){
                        current=handle()+child_idx;
                        current_idx=child_idx;
                        current_size/=2.0;
                        depth++;
                        if(coo.x)current_pos.x += current_size;
                        else current_pos.x -= current_size;
                        if(coo.y)current_pos.y += current_size;
                        else current_pos.y -= current_size;
                        if(coo.z)current_pos.z += current_size;
                        else current_pos.z -= current_size;
                    }
                    else break;
                }
                return {current_idx,current_size,current_pos,depth};
            }

            static float distance1(const vec3& a, const vec3&b){
                return max(max(abs(a.x-b.x),abs(a.y-b.y)),abs(a.z-b.z));
            }

            constexpr static inline float box(const glm::vec3& pos, float b) {
                vec3 q = abs(pos) - b;
                return length(max(q,0.0f)) + min(max(q.x,max(q.y,q.z)),0.0f);
            }

            constexpr inline Attrs operator()(glm::vec3 pos)const {
                pos-=offset;
                if(pos.x>size || pos.x<-size ||pos.y>size || pos.y<-size || pos.z>size || pos.z<-size)return {size,{}}; //return {boxdistance,current->attrs.fields};

                auto sample_1 = search(pos);
                auto base = handle();

                /*
                if(base[sample_1.idx].attrs.distance>=0){
                    if (base[sample_1.idx].attrs.distance>sample_1.size*std::numbers::sqrt2)return {(base[sample_1.idx].attrs.distance-distance1(pos,sample_1.center))/5.0f,base[sample_1.idx].attrs.normals,base[sample_1.idx].attrs.fields};
                }
                else{
                    if (-base[sample_1.idx].attrs.distance>sample_1.size*std::numbers::sqrt2)return {(base[sample_1.idx].attrs.distance+distance1(pos,sample_1.center))/5.0f,base[sample_1.idx].attrs.normals,base[sample_1.idx].attrs.fields};
                }
                return {box(pos-sample_1.center,sample_1.size),base[sample_1.idx].attrs.normals,base[sample_1.idx].attrs.fields};
                */
            if(sample_1.depth<depth+1){
                if(base[sample_1.idx].attrs.distance>=0){
                    auto d = (base[sample_1.idx].attrs.distance-distance1(pos,sample_1.center))/8.0f;
                    return {d,base[sample_1.idx].attrs.normals,base[sample_1.idx].attrs.fields};
                }
                else{
                    auto d = (base[sample_1.idx].attrs.distance+distance1(pos,sample_1.center))/8.0f;
                    return {d,base[sample_1.idx].attrs.normals,base[sample_1.idx].attrs.fields};
                }
            }
            return {box(pos-sample_1.center,sample_1.size),base[sample_1.idx].attrs.normals,base[sample_1.idx].attrs.fields};
            


               if (abs(base[sample_1.idx].attrs.distance)<sample_1.size*std::numbers::sqrt3 && sample_1.depth<8 ) return {distance(pos,sample_1.center),base[sample_1.idx].attrs.normals,base[sample_1.idx].attrs.fields};    //Point resolved
               else if (abs(base[sample_1.idx].attrs.distance)>sample_1.size*std::numbers::sqrt3 && sample_1.depth<8 ) return {base[sample_1.idx].attrs.distance-distance(pos,sample_1.center),base[sample_1.idx].attrs.normals,base[sample_1.idx].attrs.fields}; //Skip space
               else return {distance(pos,sample_1.center),base[sample_1.idx].attrs.normals,base[sample_1.idx].attrs.fields}; //to interpolate

               if (abs(base[sample_1.idx].attrs.distance)<sample_1.size*std::numbers::sqrt3 || sample_1.depth>=10 ) return {(distance(pos,sample_1.center)-base[sample_1.idx].attrs.distance)/5.0f,base[sample_1.idx].attrs.normals,base[sample_1.idx].attrs.fields};
               else return {(base[sample_1.idx].attrs.distance-distance(pos,sample_1.center))/5.0f,base[sample_1.idx].attrs.normals,base[sample_1.idx].attrs.fields};

                if(base[sample_1.idx].attrs.distance>=0){
                    if (base[sample_1.idx].attrs.distance>sample_1.size*std::numbers::sqrt3)return {(distance(pos,sample_1.center)),base[sample_1.idx].attrs.normals,base[sample_1.idx].attrs.fields};
                }
                else{
                    if (-base[sample_1.idx].attrs.distance>sample_1.size*std::numbers::sqrt3)return {(-distance(pos,sample_1.center)),base[sample_1.idx].attrs.normals,base[sample_1.idx].attrs.fields};
                }
                return {base[sample_1.idx].attrs.distance,base[sample_1.idx].attrs.normals,base[sample_1.idx].attrs.fields};

                //Interpolation attempt which was a failure
                /*decltype(sample_1) sample_2;
                for(float divider=1.0f;divider<17.0;divider*=2.0){
                    sample_2= search(pos+normalize(pos-sample_1.center)*length(pos-sample_1.center)/divider);
                    if(distance(sample_2.center,sample_1.center)<0.0001)break;
                }
                return {
                    ((base[sample_1.idx].attrs.distance-distance1(pos,sample_1.center))*distance(pos,sample_2.center)+
                     (base[sample_2.idx].attrs.distance-distance1(pos,sample_2.center))*distance(pos,sample_1.center))/
                     (distance(pos,sample_2.center)+distance(pos,sample_1.center)),
                     base[sample_1.idx].attrs+base[sample_2.idx].attrs};
                */

                /*
                
                if(base[sample_1.idx].attrs.distance>=0){
                    if (base[sample_1.idx].attrs.distance>sample_1.size*std::numbers::sqrt2)return {(base[sample_1.idx].attrs.distance-distance1(pos,sample_1.center))/4.0f,base[sample_1.idx].attrs.fields};
                    else{
                        float retdist = sample_1.size*std::numbers::sqrt2;
                        //vec3 points [3][3][3];
                        for(int x = -1; x<2; x++)
                        for(int y = -1; y<2; y++)
                        for(int z = -1; z<2; z++)
                        {                        
                            if(x==0 && y==0 && z==0)continue;
                            auto tmp = sample_1.center+vec3{x,y,z}*sample_1.size*2.0f;
                            if(distance(pos,tmp)<=distance(pos,sample_1.center)){
                                //points[x+1][y+1][z+1]=sample_1.center+vec3{x,y,z}*sample_1.size*2.0f;
                                auto factor = dot(tmp-sample_1.center,pos-sample_1.center)/length(tmp-sample_1.center);
                                auto newsample = base[search(tmp).idx];
                                if(newsample.attrs.distance<0){
                                    retdist=min(retdist, sample_1.size*(float)std::numbers::sqrt2-distance(pos,tmp));
                                }
                            }
                        }
                        auto ccfg =base[sample_1.idx].attrs.fields;
                        //ccfg.idx=5;
                        return {retdist,ccfg};
                    }

                }
                else{
                    if (-base[sample_1.idx].attrs.distance>sample_1.size*std::numbers::sqrt2)return {(base[sample_1.idx].attrs.distance+distance1(pos,sample_1.center))/4.0f,base[sample_1.idx].attrs.fields};
                    else{
                     float retdist = -sample_1.size*std::numbers::sqrt2;
                        //vec3 points [3][3][3];
                        for(int x = -1; x<2; x++)
                        for(int y = -1; y<2; y++)
                        for(int z = -1; z<2; z++)
                        {                        
                            if(x==0 && y==0 && z==0)continue;
                            auto tmp = sample_1.center+vec3{x,y,z}*sample_1.size*2.0f;
                            if(distance(pos,tmp)<=distance(pos,sample_1.center)){
                                //points[x+1][y+1][z+1]=sample_1.center+vec3{x,y,z}*sample_1.size*2.0f;
                                auto factor = dot(tmp-sample_1.center,pos-sample_1.center)/length(tmp-sample_1.center);
                                auto newsample = base[search(tmp).idx];
                                if(newsample.attrs.distance>0){
                                    retdist=max(retdist, sample_1.size*(float)std::numbers::sqrt2+distance(pos,tmp));
                                }
                            }
                        }
                        auto ccfg =base[sample_1.idx].attrs.fields;
                        //ccfg.idx=4;
                        return {retdist,ccfg};
                    }
                }
                */


            }
            constexpr inline float sample(const glm::vec3& pos)const {return operator()(pos).distance;}



            constexpr inline bbox_t bbox() const{return {vec3(-size),vec3(size)};}

            constexpr OctaSampled3D(handle_t h, Attrs::extras_t cfg={}):cfg(cfg),_handle(h){
                depth=((sampler::octatree3D::header_t*)global_shared[h].base)->max_depth;
                offset=((sampler::octatree3D::header_t*)global_shared[h].base)->offset;
                size=((sampler::octatree3D::header_t*)global_shared[h].base)->box_size;
            }

            constexpr inline void traits(traits_t& to) const{
                to.is_sym={true,true,true};
                to.is_exact_inner=true;
                to.is_exact_outer=true;
                to.is_bounded_inner=true;
                to.is_bounded_outer=true;
                to.outer_box={vec3(-size),vec3(size)};
            }

            constexpr inline static const char* _name = "OctaSampled3D";

            constexpr inline static field_t _fields[] = {
                FIELD_R(OctaSampled3D,shared_buffer,deftype,_handle, "Handle"),
            };

            constexpr inline const char* name()const{return _name;}
            constexpr inline fields_t fields()const{return {_fields,sizeof(_fields)/sizeof(field_t)};}
            constexpr inline visibility_t is_visible() const{return visibility_t::VISIBLE;}
        };
    }}

    sdf_register_primitive(OctaSampled3D);
}