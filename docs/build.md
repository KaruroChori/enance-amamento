## Requirements & toolchains

You will need a recent version of `meson` as then build system, and a proper toolchain compiled with support for OpenMP (make sure it matches your target architecture).  
At the moment, only clang-21 has been tested and is the one used for development, but 20 should work just fine.  
`gcc` is not supported yet, but preliminary work has been done to use it, please check the instructions later in this document.  
Make sure you are using a recent version like `gcc-14` or `gcc-15`. There is a special branch with offloading patches, it might be a good idea.  

If no support for OpenMP is available, there is a stub implementation in the subprojects, but it has not been tested nor integrated in the build systems as of yet.  
Also, while the main library has no external dependency, aside from those directly handled by meson, some utilities like the main UI do.  
Make sure `sdl3` is installed on your system if you want to build and run them.

## Toolchain

Sadly, versions of clang distributed via [https://apt.llvm.org/](https://apt.llvm.org/) are not good enough.  
While OpenMP and offloading are working fine, they don't distribute libraries like libc for the relevant offloaded targets.  
It is possible other distros don't share the same problem with their packages, but I have not tried myself.
This issue is currently [tracked](https://github.com/KaruroChori/enance-amamento/issues/9) upstream.  

For the time being, you will need to build your own version of clang with full support for offloading.  
Doing so can be very time consuming even if you have a decent workstation.  
I will write a document covering that at some point I guess.

## Building

Instead of working directly with meson, you can use the provided `Makefile` to run simplified commands.  
Just remember to configure your environment based on your needs/machine.  
Check `Makefile` for more details.
If you don't have `clang-21` installed you might have to change the default `platform` file.

For example, on my system:

```bash
make run OFFLOAD=nvptx64
```

If you want to use your CPU as offloaded device

```bash
make run OFFLOAD=amd64-linux-unknown #Or match your architecture.
```

If you want to build a version without omp support at all (it will be very slow)

```bash
make run NO_OMP=true
```

### GCC

Support for GCC is not fully there yet, but you can try to compile it using the following (in case of nvidia devices):

```bash
make run TOOLCHAIN=platforms/gcc.ini OFFLOAD=nvptx-none
```

## Library configuration

The behaviour of the library can be controlled using some preprocessor variables:
- `SDF_DEFAULT_ATTRS` sets an alternative to `idx_attrs<true>` as default attributes for the SDF being computed.
- `SDF_HEADLESS` set to `true` is used for situations where no serialization is supported (`to_xml` and similar) and no shared buffers are allocated. Useful for tests.
- `SDF_IS_HOST` is used to specify if this compiled code should run on the host device or on an offloaded one.  
  It should not be manually set, your build system is meant to configure it for you z.B. passing `-D SDF_IS_HOST=true -Xopenmp-target -D SDF_IS_HOST=false` as args.