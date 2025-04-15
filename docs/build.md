## Requirements & toolchains

You will need a recent version of `meson` as then build system, and a proper toolchain compiled with support for OpenMP (make sure it matches your target architecture).  
At the moment, only clang-21 has been tested and is the one used for development, but 20 should work just fine.  
`gcc` is not supported/tested, and would require some changes in the build script as its arguments are not the same as for `clang`.  
If you really want, make sure you are using the special branch with offloading patches for `gcc-14` or `gcc-15`.  

If no support for OpenMP is available, there is a stub implementation in the subprojects, but it has not been tested nor integrated in the build systems as of yet.  
Also, while the main library has no external dependency aside from those directly handled by meson, some utilities like the main UI do.  
Make sure `sdl3` is installed on your system if you want to build and run them.

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
make run OFFLOAD=amd64 #Or match your architecture.
```

If you want to build a version without omp support at all (it will be very slow)

```bash
make run NO_OMP=true
```

## Library configuration

The behaviour of the library can be controlled using some preprocessor variables:
- `SDF_DEFAULT_ATTRS` sets an alternative to `idx_attrs<true>` as default attributes for the SDF being computed.
- `SDF_HEADLESS` set to `true` is used for situations where no serialization is supported (`to_xml` and similar) and no shared buffers are allocated. Useful for tests.
- `SDF_IS_HOST` is used to specify if this compiled code should run on the host device or on an offloaded one.  
  It should not be manually set, your build system is meant to configure it for you z.B. passing `-D SDF_IS_HOST=true -Xopenmp-target -D SDF_IS_HOST=false` as args.