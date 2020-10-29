# IntegralRayKernelBase

## Description

`IntegralRayKernelBase` is the base class for computing an integral along a [Ray.md] segment. It is used as a parent for classes like [IntegralRayKernel.md], [RayKernel.md], and [ADRayKernel.md].

It makes available the standard [Assembly.md] members like `_q_point` for the current quadrature points and `_JxW` for the current weights. It overrides the standard `onSegment()` behavior of a [RayKernel](RayKernels/index.md) and replaces it with a `computeIntegral()` method to be overriden to compute the integral on a segment.
