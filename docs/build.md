You will need a recent version of `meson` as a build system, and a proper toolchain compiled with support for OpenMP (make sure it matches your target architecture).  
At the moment, only clang-21 has been tested and is the one used for development, but 20 should work just fine.  
`gcc` is not supported/tested, and would require some changes in the build script as its arguments are not the same as for `clang`.  
If you really want, make sure you are using the special branch with offloading patches for `gcc-14` or `gcc-15`.  

If no support for OpenMP is available, there is a stub implementation in the subprojects, but it has not been tested nor integrated in the build systems as of yet.  
Also, while the main library has no external dependency aside from those directly handled by meson, some utilities like the main UI do.  
Make sure `sdl3` is installed on your system if you want to build and run them.

Instead of working directly with meson, you can use the provided `Makefile` to run simplified commands.  
Just remember to configure your environment based on your needs/machine.  
If you don't have `clang-21` installed you might have to change the default `platform` file.

For example, on my system:

```bash
make run OFFLOAD=nvptx64
```

The application supports the following controls:
- Right click for contextual menu
- `shift` + `left-click` + movement is panning
- `ctrl` + `left-click` + movement is rotating
- `middle-click` to set the pivot point
- `scroll` to move in or out.
- Numpad digits to rotate at fixed positions.
- `home` to reset all camera tranforms.
- `esc` to toggle the overlay menu.

If these shortcuts are not working, make sure you don't have an imgui window selected, as that will take focus.