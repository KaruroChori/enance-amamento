Most of the modern C++ features will work in the offloaded regions, with some restrictions:
- Only libc is available for offloaded regions. A limited subset of the STL can also work, like `pairs`. 
- No exceptions or assertions (but static assertions are fine).
- Be mindful of bitfields. The interface between main host and devices has some [issues](https://github.com/llvm/llvm-project/issues/127334).
- Nvidia is not supporting self-referential initializers as discussed [here](https://github.com/llvm/llvm-project/issues/132429#issuecomment-2760069764). 