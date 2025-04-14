> The interpreted tree is slow.

Yes they are. Arbitrary recursion on GPU is much slower compared to programs in which the stack structure can be resolved statically.  
To fix this issue there are three non-exclusive paths:
- To compute parts of the tree in a more efficient format, like Octree or properly compiling it at runtime.
- To limit recursion, introducing some form of tape-machine which is able to unroll the tree. Still interpreted but much faster.
- To introduce more fixed pipelines, like add transforms and visibility toggles directly in the SDF state, flat lists for associative operators etc.

To be fast, the final application based on this library should use one or more of the above depending on its scope and architecture.

> Regardless, rendering is slow.

We are not leveraging the hardware-based rendering capabilities of you GPU. So yes, that type of optimization and the raytraced nature of the algorithms involved make this implementation computationally expensive. And also the fact code is not fully optimized.

> Why OpenMP? Why not simply vulkan?

For portability. This code is nothing special, the library can safely run on very small ARM processors with next to no memory, and scale up to a single CPU with 20 GPUs attached.

