This section of the library provides all the basic SDF functionality.

- **modifiers** are special wrappers which can affect the behaviour of a wide range of SDFs. For example using the bounding box to operate as a LOD.
- **operators**
- **primitives**

## Types of SDFs

- `comptime` resolved as statically typed const expressions, with static dispatching (fastest)
- `dynamic` resolved as statically typed const expressions, with dynamic dispatching. (not meant for normal usage, internally used by polymorphic)
- `polymorphic` resolved as reference counted, type dynamically resolved. (slower, for CPU representation and editing)
- `octree`
- And `tree_idx` resolved as an explicit tree structure on a linear memory layout, type dynamically resolved. (slowest, for GPU offloading before compiled versions are available)

Regardless of the type of primitive, their interface and usage is virtually the same when constructing expressions.
