#include <cmath>
#include <omp.h>
#include <print>
#include <chrono>

auto slice_work = [](size_t device, float start, float end){
    //TODO: This will fail on clang because of https://github.com/llvm/llvm-project/issues/136652
    #pragma omp target teams device(device) 
    {

        /*#pragma omp distribute parallel for collapse(2) schedule(static,1)
        for (int i = 0; i < render_height; i++) {
            for (int j = 0; j < render_width; j++) {
                vec2 coo = (vec2{j,i}*scale-0.5f*vec2{display_width,display_height})/(float)display_height;
                layer_0[i*render_width+j]= scene.render(coo);
            }
        }*/
    }
};

int main(){
    const size_t devnum = omp_get_num_devices();
    struct{
        float speed;
        float weight;
        float max_memory;
    } devices[devnum];
    float integral_weights[devnum];

    float lambda = 0.1;

    for(auto& device:devices)device={1.0f,1.0f/(float)devnum, INFINITY};

    /*
        Basically we have to solve the optimization problem:
            arg_min(weight_i) for max_i(speed_i*weight_i) constrained to sum_i(total_work*weight_i)=total_work
        Where speed_i is an estimate based on prior runs, or a seeded value.
        We also have a secondary (softer) constraint to consider:
            weight_i*total_work < allocable_memory_i
    */

    while(true){

        //Setup the integral array to alway know absolute slices
        if(devnum>0)integral_weights[0]=devices[0].weight;
        for(size_t i = 1; i<devnum ; i++){
            integral_weights[i]=devices[i].weight+integral_weights[i-1];
        }

        float total_speed = 0;
        #pragma omp parallel for reduction(+:total_speed)
        for(size_t i = 0; i<devnum ; i++){
            //std::print("{}\n",omp_get_thread_num());
            auto start = std::chrono::high_resolution_clock::now();

            slice_work(i,(i>0?integral_weights[i-1]:0),integral_weights[i]);

            auto end = std::chrono::high_resolution_clock::now();
            auto tmp_speed = devices[i].weight/(end-start).count();
            devices[i].speed=tmp_speed;
            total_speed+=tmp_speed;
        }

        //#pragma omp parallel for
        for(size_t i = 1; i<devnum ; i++){
            devices[i].weight=(1.f-lambda)*devices[i].weight+lambda*(devices[i].speed/total_speed);
        }

        //TODO: Implement memory constraints
    }
    return 0;
}