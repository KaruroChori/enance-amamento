#include <cmath>
#include <omp.h>
#include <print>
#include <chrono>

#pragma omp requires unified_shared_memory


//Just annoying workaround without lambdas to avoid https://github.com/llvm/llvm-project/issues/136652
constexpr size_t HEIGHT=40000, WIDTH=80000;
static uint8_t* data;

auto slice_work(size_t device, float start, float end){
    if(data==nullptr){
        std::print("oh nooooo\n");
        throw "NOOOO";
    }
    #pragma omp target teams device(device) 
    {

        #pragma omp distribute parallel for collapse(2) schedule(static,1)
        for (int i = (int)(HEIGHT*start); i < (int)(HEIGHT*end); i++) {
            for (int j = 0; j < WIDTH; j++) {
                data[i*WIDTH+j]+=std::sqrt(i+j*i)+std::sqrt(j+j*i)+std::pow(j,i);
            }
        }
    }

    return;
};

int main(){
    data= (uint8_t*) omp_alloc(HEIGHT*WIDTH);
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
        //Cannot use this one. It would be nice to keep code more regular, but it requires OMP_WAIT_POLICY=passive not to be very wasteful. 
        // So I will be using the std::async version I guess, since that policy cannot be set for each instance separately.
        #pragma omp parallel for reduction(+:total_speed) schedule(dynamic,1)
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
        for(size_t i = 0; i<devnum ; i++){
            devices[i].weight=(1.f-lambda)*devices[i].weight+lambda*(devices[i].speed/total_speed);
            printf("%ld %f\n",i, devices[i].weight);
        }

        //TODO: Implement memory constraints
    }

    omp_free(data);
    return 0;
}


/*

int main(){
    data= (uint8_t*) omp_alloc(HEIGHT*WIDTH);
    const size_t devnum = omp_get_num_devices();
    struct{
        float speed;
        float weight;
        float max_memory;
    } devices[devnum];
    float integral_weights[devnum];

    float lambda = 0.1;

    for(auto& device:devices)device={1.0f,1.0f/(float)devnum, INFINITY};


    while(true){
        std::future<float> futures[devnum];
        //Setup the integral array to alway know absolute slices
        if(devnum>0)integral_weights[0]=devices[0].weight;
        for(size_t i = 1; i<devnum ; i++){
            integral_weights[i]=devices[i].weight+integral_weights[i-1];
        }

        float total_speed = 0;
        //#pragma omp parallel for reduction(+:total_speed) schedule(dynamic,1)
        for(size_t i = 0; i<devnum ; i++){
            futures[i] = std::async(std::launch::async,[&,i](){
                auto start = std::chrono::high_resolution_clock::now();

                slice_work(i,(i>0?integral_weights[i-1]:0),integral_weights[i]);
    
                auto end = std::chrono::high_resolution_clock::now();
                auto tmp_speed = devices[i].weight/(end-start).count();
                return tmp_speed;
            });
            //std::print("{}\n",omp_get_thread_num());
        }

        {
            int w = 0;
            for (auto &fut : futures) {
                auto v = fut.get();
                devices[w].speed=v;
                total_speed+=v;
                w++;
            }
        }
        
        //#pragma omp parallel for
        for(size_t i = 0; i<devnum ; i++){
            devices[i].weight=(1.f-lambda)*devices[i].weight+lambda*(devices[i].speed/total_speed);
            printf("%ld %f\n",i, devices[i].weight);
        }

        //TODO: Implement memory constraints
    }

    omp_free(data);
    return 0;
}
*/