#include <cmath>
#include <cstddef>
#include <future>
#include <omp.h>
#include <print>
#include <chrono>

#pragma omp requires unified_shared_memory

//Just annoying workaround without lambdas to avoid https://github.com/llvm/llvm-project/issues/136652

template<typename T>
concept algorithm_i = requires(const T& self, typename T::slice_t slice, size_t device, const std::vector<float>& iweights){
    //std::is_same<decltype(self.distance),float>();
    //std::is_same<decltype(self.fields),typename T::extras_t>();
    {self(device,slice)} -> std::same_as<void> ;
    {self.slices(iweights)} -> std::same_as<std::vector<typename T::slice_t>> ;
};

struct algorithm_t{
    size_t height=40000, width=80000;
    uint8_t* data;


    struct slice_t{
        size_t start;
        size_t end;
    };

    //This current demo implementation is not correct, as some rows will be duplicated. For now that is ok, it is just for testing purposes.
    std::vector<slice_t> slices(const std::vector<float>& integral_weights) const{
        std::vector<slice_t> tmp;
        tmp.reserve(integral_weights.size());

        for(size_t i = 0; i<integral_weights.size(); i++){
            tmp.push_back({(size_t)((i>0?integral_weights[i-1]:0)*height),(size_t)(integral_weights[i]*height)});

        }
        return tmp;
    }

    void operator()(size_t device, slice_t slice) const{
        #pragma omp target teams device(device) 
        {
    
            #pragma omp distribute parallel for collapse(2) schedule(static,1)
            for (size_t i = slice.start; i < slice.end; i++) {
                for (size_t j = 0; j < width; j++) {
                    data[i*width+j]+=std::sqrt(i+j*i)+std::sqrt(j+j*i)+std::pow(j,i);
                }
            }
        }
    
        return;
    }

    algorithm_t(size_t height, size_t width):height(height),width(width){
        data=(uint8_t*)omp_alloc(height*width);
        if(data==nullptr){
            std::print("oh nooooo\n");
            throw "NOOOO";
        }
    }

    ~algorithm_t(){
        omp_free(data);
    }
};

template<algorithm_i T>
struct slicer_t{
    struct device_t{
        float speed;
        float weight;
        float max_memory;
    };

    size_t devices_n;
    std::vector<device_t> devices;
    std::vector<float> integral_weights;
    const T& op;

    float lambda;

    slicer_t(const T& op, float lambda=0.1f):op(op),lambda(lambda){
        devices_n = omp_get_num_devices();
        devices.resize(devices_n);
        integral_weights.resize(devices_n);

        for(auto& device:devices)device={1.0f,1.0f/(float)devices_n, INFINITY};
    }

    void operator()(){
        std::future<float> futures[devices.size()];

        //Setup the integral array to alway know absolute slices
        if(devices_n>0)integral_weights[0]=devices[0].weight;
        for(size_t i = 1; i<devices_n ; i++){
            integral_weights[i]=devices[i].weight+integral_weights[i-1];
        }

        auto slices = op.slices(integral_weights);

        for(size_t i = 0; i<devices_n ; i++){

            futures[i] = std::async(std::launch::async,[&,i](){
                auto start = std::chrono::high_resolution_clock::now();

                op(i,slices[i]);
    
                auto end = std::chrono::high_resolution_clock::now();
                auto tmp_speed = devices[i].weight/(end-start).count();
                return tmp_speed;
            });
        }


        float total_speed = 0;
        {
            int w = 0;
            for (auto &future : futures) {
                auto v = future.get();
                devices[w].speed=v;
                total_speed+=v;
                w++;
            }
        }
        
        for(size_t i = 0; i<devices_n ; i++){
            devices[i].weight=(1.f-lambda)*devices[i].weight+lambda*(devices[i].speed/total_speed);
            printf("%ld %f %d\n",i, devices[i].weight, op.data[5]);
        }

        //TODO: Implement memory constraints
    }
};


int main(){
    algorithm_t instance(40000,80000);

    slicer_t slicer(instance);
    while(true){
        slicer();
    }

    return 0;
}
