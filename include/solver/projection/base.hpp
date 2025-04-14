#pragma once

/**
 * @file base.hpp
 * @author karurochari
 * @brief Basic interface of a projection 2D solver.
 * @date 2025-03-09
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "sdf/commons.hpp"


namespace solver{
namespace projection{

using namespace glm;

struct base_camera_t{
    glm::vec3 pos = {0,2,-5};
    glm::vec3 rot = { 0,0,0};
    float zoom = 0.0;
    enum projection_t{ORTHO,PERPSECTIVE} projection = PERPSECTIVE;

};

struct screen_camera_t:base_camera_t{
    int canvas_width, canvas_height;
    float resolution_scale = 1.0;
};

//TODO: add arg to define if the source SDF is assumed to be exact,bounded or neither. It will switch a bit the logic of the ray caster.

template<sdf::sdf_i SDF>
struct base{
    SDF sdf;

    base_camera_t camera = {.pos={0,1,0},.zoom=1.0};

    vec2 mouse_pos = {0,0};
    vec3 sun_pos = {0,5,6};
    vec4 sky = {1.0,0.9,0.9,1.0};

    base(const SDF& sdf,const vec3& camera_pos):sdf(sdf){camera.pos=camera_pos;}
    base(const SDF& sdf):sdf(sdf){}

    constexpr static int MAX_STEPS = 500;
    constexpr static float MAX_DIST = 300;
    constexpr static float SURFACE_DIST = sdf::EPS;

    struct output_t : SDF::attrs_t::extras_t {
        float   depth;
        vec3    normals;
        uint32  iterations;
    };

    std::pair<typename SDF::attrs_t, uint> march(vec3 ro, vec3 rd, float d0=0.0f){
        auto [a,b] = march_schnell(ro,rd,d0);
        //Technically this adds one more computation step, however it reduces the cost of computing fields for all the intermediate steps, so things will be better for more complex scenes.
        auto tmp = sdf(ro+a*rd);
        return {{a,tmp.normals,tmp.fields},b};
    }

    //Reduced version to avoid spending too much space on useless args.
    std::pair<float, uint> march_schnell(vec3 ro, vec3 rd, float d0=0.0f){
        int i=0;
        for(;i<MAX_STEPS;i++){
            vec3 p = ro+d0*rd;
            auto dS = sdf.sample(p);
            d0+=dS;
            if(abs(dS)<SURFACE_DIST) break;
            if(d0>MAX_DIST) {d0=INFINITY;i=MAX_STEPS;break;}
        }
        return {d0,i};
    }

    //TODO: stop when the sampled value is smaller compared to the radius of the cone at that point.
    std::pair<float, uint> march_cone_schnell(vec3 ro, vec3 rd, float r, float d0=0.0f){
        int i=0;
        for(;i<MAX_STEPS;i++){
            vec3 p = ro+d0*rd;
            auto dS = sdf.sample(p);
            if(abs(dS)<d0*r) break;
            else d0+=dS;
            if(d0>MAX_DIST) {d0=INFINITY;i=MAX_STEPS;break;}
        }
        return {d0,i};
    }

    float light(vec3 p, vec3 n){
        vec3 l = normalize(sun_pos-p);
        float diffuse = dot(n,l);
        float shadow_distance = march_schnell(p+n*SURFACE_DIST*1.05f,l).first;
        if(shadow_distance<length(sun_pos-p)) return diffuse*0.2;
        return diffuse;
    }

    mat2 rot2D(float angle){
        float s = sin(angle);
        float c = cos(angle);
        return mat2(c,-s,s,c);
    }

    std::pair<float,uint> render_schnell(vec2 uv, float hint = 0.0f){
        uv.y=-uv.y;
        vec3 rd = normalize(vec3(uv * (camera.zoom+1.0f),1));
        vec3 ro = camera.pos;

        {auto rot = rot2D(-camera.rot.y);auto td = rd.yz()*rot;rd.y=td.x;rd.z=td.y;}
        {auto rot = rot2D(-camera.rot.x);auto td = rd.xz()*rot;rd.x=td.x;rd.z=td.y;}
        {auto rot = rot2D(-camera.rot.z);auto td = rd.xy()*rot;rd.x=td.x;rd.y=td.y;}

        return march_schnell(ro,rd,hint);
    }

    std::pair<float,uint> render_cone_schnell(vec2 uv, float radius, float hint = 0.0f){
        uv.y=-uv.y;
        vec3 rd = normalize(vec3(uv * (camera.zoom+1.0f),1));
        vec3 ro = camera.pos;

        {auto rot = rot2D(-camera.rot.y);auto td = rd.yz()*rot;rd.y=td.x;rd.z=td.y;}
        {auto rot = rot2D(-camera.rot.x);auto td = rd.xz()*rot;rd.x=td.x;rd.z=td.y;}
        {auto rot = rot2D(-camera.rot.z);auto td = rd.xy()*rot;rd.x=td.x;rd.y=td.y;}

        return march_cone_schnell(ro,rd,radius, hint);
    }

    output_t render(vec2 uv, float hint = 0.0f){
        uv.y=-uv.y;
        vec3 rd = normalize(vec3(uv * (camera.zoom+1.0f),1));
        vec3 ro = camera.pos;

        {auto rot = rot2D(-camera.rot.y);auto td = rd.yz()*rot;rd.y=td.x;rd.z=td.y;}
        {auto rot = rot2D(-camera.rot.x);auto td = rd.xz()*rot;rd.x=td.x;rd.z=td.y;}
        {auto rot = rot2D(-camera.rot.z);auto td = rd.xy()*rot;rd.x=td.x;rd.y=td.y;}


        auto [d,i] = march(ro,rd,hint);
        //vec3 p = ro + rd*d.distance;

        //TODO: not infinity or it breaks computation of sobel there. Replace manually added value with a SKY const, issue @ https://github.com/llvm/llvm-project/issues/132342.
        if(d.distance>MAX_DIST)return {{.uid=0,.gid=511,.idx=0,}/*SDF::attrs_t::SKY*/,MAX_DIST,d.normals,i}; 
        return {{d.fields},d.distance,d.normals,i};//.light=vec3(light(p,d.normals))
    }

    vec3 raycast(vec2 uv, float hint = 0.0f){
        uv.y=-uv.y;

        vec3 rd = normalize(vec3(uv * (camera.zoom+1.0f),1));
        vec3 ro = camera.pos;

        {auto rot = rot2D(-camera.rot.y);auto td = rd.yz()*rot;rd.y=td.x;rd.z=td.y;}
        {auto rot = rot2D(-camera.rot.x);auto td = rd.xz()*rot;rd.x=td.x;rd.z=td.y;}
        {auto rot = rot2D(-camera.rot.z);auto td = rd.xy()*rot;rd.x=td.x;rd.y=td.y;}

        auto [d,i] = march_schnell(ro,rd,hint);
        vec3 p = ro + rd*d;
        return p;
    }
};


}
}
