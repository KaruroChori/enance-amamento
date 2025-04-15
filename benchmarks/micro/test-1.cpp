#define ANKERL_NANOBENCH_IMPLEMENT
#include <nanobench.h>
#define SDF_HEADLESS true
#include <sdf/sdf.hpp>
#include <glm/glm.hpp>

int main() {
    using namespace sdf::comptime;
    auto sdf_a = Sphere({{0,3,2}})+glm::vec3{0,0,2}+Sphere({{0,3,1}});

    {
        double d = 1.0;
        ankerl::nanobench::Bench().minEpochIterations(1).run("some double ops", [&] {
            for(int i=0;i<4000;i++)
                for(int j=0;j<2000;j++)
                    d+=sdf_a({i,j,0.0}).distance;
            ankerl::nanobench::doNotOptimizeAway(d);
        });
    }

    {
        double d = 1.0;
        ankerl::nanobench::Bench().minEpochIterations(1).run("some double ops", [&] {
            #pragma omp parallel for simd collapse(2) reduction(+:d)
            for(int i=0;i<4000;i++)
                for(int j=0;j<2000;j++)
                    d+=sdf_a({i,j,0.0}).distance;
            ankerl::nanobench::doNotOptimizeAway(d);
        });
    }

    {
        double d = 1.0;
        ankerl::nanobench::Bench().minEpochIterations(1).run("some double ops", [&] {
            #pragma omp teams distribute parallel for  collapse(2) reduction(+:d)
            for(int i=0;i<4000;i++)
                for(int j=0;j<2000;j++)
                    d+=sdf_a({i,j,0.0}).distance;
            ankerl::nanobench::doNotOptimizeAway(d);
        });
    }

    {
        double d = 1.0;
        ankerl::nanobench::Bench().minEpochIterations(1).run("some double ops", [&] {
            #pragma omp target teams distribute parallel for collapse(2) reduction(+:d)
            for(int i=0;i<4000;i++)
                for(int j=0;j<2000;j++)
                    d+=sdf_a({i,j,0.0}).distance;
            ankerl::nanobench::doNotOptimizeAway(d);
        });
    }
}