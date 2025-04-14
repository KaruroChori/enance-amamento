This section of the library provides all the basic SDF functionality.

- **modifiers** are special wrappers which can affect the behaviour of a wide range of SDFs. For example using the bounding box to operate as a LOD.
- **operators**
- **primitives**
- **special**

## SDFs representations

- `comptime` resolved as statically typed const expressions, with static dispatching (fastest)
- `dynamic` resolved as statically typed const expressions, with dynamic dispatching. (not really meant for normal scenarios, internally used by polymorphic)
- `polymorphic` resolved as reference counted, type dynamically resolved. (slower, for CPU representation and editing)
- `octa-tree` (both 2D and 3D variants) computed from a dynamically loaded data source. (fast)
- `interpreted` (based on `tree-idx`) resolved as an explicit tree structure on a linear memory layout, dynamically computed and resolved. (slowest, only meant as placeholder before a compiled version is made available)
- `dynlib` resolved from a dynamic library, usually built as comptime. It requires runtime support for generation. (faster)

| **Name**    | **Sampling speed** | **Generation speed** | **Memory**  | **Mutable (Structure)** | **Mutable (Fields)** | **Serializable** |
|-------------|--------------------|----------------------|-------------|-------------------------|----------------------|------------------|
| comptime    | fastest            | compile time         | smallest    | no                      | yes                  | yes (indirect)   |
| dynamic     | faster             | compile time         | small       | no (in general)         | yes                  | yes              |
| polymorphic | fast               | fast                 | small       | yes                     | yes                  | yes              |
| octa-tree   | fast               | slowest[^1]          | highest[^1] | no                      | no                   | no               |
| interpreted | slowest            | fast                 | small       | in theory               | yes                  | yes              |
| dynlib      | faster             | slow                 | average[^2] | no                      | yes                  | yes (indirect)   |

Regardless of the type of primitive, their interface and usage is virtually the same when constructing expressions.

[^1]: The complexity of the octa-tree depends on the details of the surfaces involved. For example, fractal surfaces would be much more expensive in steps and space compared to a box of equivalent volume. While it is technically possible to generate an SDF expression more expensive compared to its octa-tree, in virtually any practical scenario that will not be the case, and the difference is likely going to be order of magnitudes different. However, the maximum amount of time needed for rendering a frame is bounded based on its depth, so frame times can be very predictable.

[^2]: A dynlib has the same linear complexity in space of a comptime expression, but being distributed as a shared library it comes with a lot of bloat. How much depends on the compiling profile adopted, as we can trade time of generation for memory avoiding some expensive optimizations like `-lto`.
