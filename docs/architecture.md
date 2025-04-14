This project contains four separate packages:

- `enance-sdf`, the library of signed distance fields. `sdf/sdf.hpp`
- `enance-ui`, a basic UI library based on SDL3 and imgui, used as front end for the editor. `ui/ui.hpp`
- `enance-edit`, the actual editor. Potentially headless.
- `enance`, the visual editor based on `enance-edit` and `enance-ui`.

```mermaid
graph TD
    %% Nodes
    enance-sdf[enance-sdf]
    enance-ui[enance-ui]
    enance-edit[enance-edit]
    enance[enance]
    imgui[imgui]
    sdl3[sdl3]
    pugixml[pugixml]
    vs-templ[vs.templ]

    %% Dependencies
    %% enance-sdf is the root (child -> parent not applicable)
    enance-ui --> enance-sdf
    enance-ui --> imgui
    enance-ui --> sdl3

    enance-edit --> enance-sdf
    enance-edit --> pugixml
    enance-edit --> vs-templ

    enance --> enance-edit
    enance --> enance-ui

    click vs-templ "https://github.com/lazy-eggplant/vs.templ"
    click pugixml "https://github.com/zeux/pugixml"
    click imgui "https://github.com/ocornut/imgui"
    click sdl3 "https://wiki.libsdl.org/SDL3/FrontPage"
```
