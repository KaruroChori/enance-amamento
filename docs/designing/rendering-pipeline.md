Features wanted from the demo renderer:

- [ ] Orthogonal projection
- [x] and Perspective projections
- [ ] Some form of anti-aliasing
- [ ] ~~Support for two lights, a sun and a point-light attached to the camera~~ no real light for the demo renderer
- [ ] Ambient occlusion
- [x] Indexed based material
- [ ] Transparency (hash and exact (blended?))
- [x] Edges highlight / border (to be extended)
- [x] Material feathering (hash)

## Output

First pass:

- Depth
- Normals
- Material index
- ~~Light contributions for each group~~

Second pass:

- Computing sobol filter (grayscale gradient, depth gradient, and the two "normalized" versions)

Third pass:

- Computing sobol dilation to create the visible edge

Fourth pass:

- Final color/material composition.

Fifth pass:

- Any screenspace transform needing a color domain like, like DoF, fog effects, bloom, etc. Not supported for now.

## GID/UID

- `0/*` global base group. Nothing special.
- `511/*` sky group. Special.
- `*/0` part of the group but fake address. Basically it cannot be addressed externally

## Static + Dynamic

We might want to split computation between a static tree and a dynamic subtree.  
Basically we use two split framebuffers, where one is updated at each frame; the other is kept unless the scene state has changed (moving camera etc.)