> The interpreted tree is slow.

Yes they are. Arbitrary recursion on GPU is much slower compared to programs in which the stack structure can be resolved statically.  
To fix this issue there are three non-exclusive paths:
- To compute parts of the tree in a more efficient format, like Octree or properly compiling it at runtime.
- To limit recursion, introducing some form of tape-machine which is able to unroll the tree. Still interpreted but much faster.
- To introduce more fixed pipelines, like add transforms and visibility toggles directly in the SDF state, flat lists for associative operators etc.

To be fast, the final application based on this library should use one or more of the above depending on its scope and architecture.

> Regardless, rendering is slow.

We are not leveraging the hardware-based rendering capabilities of modern GPUs.  
So yes, the raytraced nature of the algorithms and its lack of hardware support makes the final implementation computationally expensive.  
This is actually fine. Just because rendering an SDF via raymarching is slow, it does not mean we cannot generate more compute-friendly representations.  
Many operations are so cheap for SDF that they offset the complexity of any sampling needed to construct those alternative representations.

> Why OpenMP? Why not simply vulkan?

For portability. This code is nothing special, the library can safely run on very small ARM processors with next to no memory, and scale up to multi-socket motherboards with 20 GPUs attached.

> Compiling for nvidia cards triggers some warnings 

```
nvlink warning : Stack size for entry function '__omp_offloading_fd00_7445f2a__ZN8pipeline4demoIN3sdf12_GLOBAL__N_14impl11InterpretedINS1_9idx_attrsILb1EEEEELb0EE7raycastERKN3glm3vecILi2EfLNS9_9qualifierE0EEE_l140' cannot be statically determined
```
or something of that sort. Yes that is correct, we are defining kernels for which the stack size is not known at compile time. As some point I will look for a way to silence those warnings.