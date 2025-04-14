There are two approaches:

- One where the position of the viewer is used to determine which SDF in a set to use. This depends on the camera model used, and the type of sampling needed. Still, for a game engine it is also the one which can more aggressively simplify computation, so it does make sense in some scenarios.
- One where only the position I am sampling at is used. Basically its distance from the center is used to determine if a simpler SDF should be used. If properly implemented it results in a bounded SDF and not an exact one, but the increase in iterations needed to converge might be negligible compared to how much computation is avoided and really pay off. This method is much more flexible and can be built more or less agnostic towards the SDF itself.

LOD support will be based on an off-tree temporary storage of computer bounding boxes, and a utility wrapper for SDF capable of using that value to perform early pruning.
