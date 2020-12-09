# AuxRayKernel

## Description

`AuxRayKernel` is the base class for contributing to an [AuxVariable](syntax/AuxVariables/index.md) (of type `CONSTANT MONOMIAL` only) from a [Ray.md] segment. An example is [RayDistanceAux.md], which accumulates the distance traversed by each [Ray.md] segment into an `AuxVariable` for the element that the segment is in.

To use, override the `onSegment()` method and use the `addValue()` method to add to the [AuxVariable](syntax/AuxVariables/index.md). Many useful member variables exist that describe the [Ray.md] segment - see [Using a RayKernel](syntax/RayKernels/index.md#using-a-raykernel).
