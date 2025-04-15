This document provides some high level details about features and milestones for this project.

- [ ] The calculation of bounding boxes for boolean operators are likely not correct. ~~Add support for the new traits as well.~~done
- [ ] At the moment normals are represented as a vec3, but since normalized, it would make sense to reduce it to a vec3 for storage.
- [ ] It would be nice to add an argument in meson to only handle the library and not the optional applications and CLI utils. This would also decrease a lot the number of dependencies to be installed.

## Renderer

- [ ] Styleblit fully implemented?
- [ ] [Cone marching](https://www.fulcrum-demo.org/wp-content/uploads/2012/04/Cone_Marching_Mandelbox_by_Seven_Fulcrum_LongVersion.pdf)
- [ ] The cone marching infrastructure can also be used to build a distance invariant border for the visualized sdf. It will be nice to experiment with that.

## Demo UI

- [ ] Consider adding an embedded [editor](https://github.com/Rezonality/zep) to the UI to support runtime changes to the lua script of animations.
