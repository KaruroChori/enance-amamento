Most of the modern C++ features will work in the offloaded regions, with some restrictions:
- Only libc is available for offloaded regions.  
  A limited subset of the STL can also work, but in general no libc++.  
  Regardless, it is best to write offloaded code to be free-standing whenever possible (but the header-only portions of libraries are ok)
- No exceptions, those need by library hooks must resolve in some trap.
- Be mindful of bit-fields. The interface between main host and devices has some [issues](https://github.com/llvm/llvm-project/issues/127334).
- Nvidia is not supporting self-referential initializers as discussed [here](https://github.com/llvm/llvm-project/issues/132429#issuecomment-2760069764). 
- Make sure to guard code which cannot run on the target device by using the `SDF_IS_HOST` preprocessor variable as check.