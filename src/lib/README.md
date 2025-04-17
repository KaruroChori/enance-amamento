Libraries composing `enance-amamento`. They are to be linked separately in final applications based on which features are needed.
- `sdf` offers the core functionality of the engine with all the offloadable code.
- `ui` is host only and provides a boilerplate configurable client. It has little to no dependency on `sdf`.
- `scene-import` is the XML and Lua headless frontend to load/save scenes in the demo format.