# FVInterpolationMethod

## Description

`FVInterpolationMethod` objects define lightweight policies for interpolating cell-centered data to
finite-volume faces. They are regular `MooseObject`s stored in the application's warehouse so they
can be declared once and shared by any kernel that needs them. Each method provides a small callable
handle that MOOSE kernels can cache, which keeps per-face evaluations free of virtual dispatch and
friendly to upcoming vectorized/GPU paths.

Interpolation methods are added in the `[InterpolationMethods]` block of an input file and
referenced through the `coeff_interp_method` parameter on kernels such as
[`LinearFVDiffusion`](../linearfvkernels/LinearFVDiffusion.md). If the parameter is omitted the
kernel will keep evaluating the functor directly on the face, preserving the legacy behavior.

## Available methods

- Coefficient/face interpolation policies (e.g. geometric or harmonic averaging)
- Advected interpolation policies for advection terms:
  - [FVAdvectedMinmodWeightBased.md]
  - [FVAdvectedVanLeerWeightBased.md]

## Example input syntax

```
[InterpolationMethods]
  [geom]
    type = GeometricAverage
  []
  [harm]
    type = harmonicAverage
  []
[]

[LinearFVKernels]
  [diff]
    type = LinearFVDiffusion
    variable = u
    diffusion_coeff = k
    coeff_interp_method = harm
  []
[]
```

The example above declares two interpolation schemes: a geometric (linear) average and an
inverse-distance-weighted average with exponent one. The diffusion kernel selects the IDW method at
runtime via its `coeff_interp_method` parameter.
