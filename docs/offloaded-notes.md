Most of the modern C++ features will work in the offloaded regions, with some restrictions:
- Only libc is available for offloaded regions.  
  A limited subset of the STL can also work, but in general no libc++.  
  Regardless, it is best to write offloaded code to be mostly free-standing whenever possible (but header-only libraries like glm are ok)
- No exceptions, those needed by library hooks must resolve in some trap.
- Be mindful of bit-fields. The interface between main host and devices has some [issues](https://github.com/llvm/llvm-project/issues/127334) on clang.
- There are problems with [globally defined lambdas](https://github.com/llvm/llvm-project/issues/136652) and omp pragmas on clang.
- Nvidia is not supporting self-referential initializers as discussed [here](https://github.com/llvm/llvm-project/issues/132429#issuecomment-2760069764). 
- Make sure to guard code which cannot run on the target device by using the `SDF_IS_HOST` preprocessor variable as check.