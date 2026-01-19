# FVInterpolationMethod

## Description

`FVInterpolationMethod` objects define lightweight policies for interpolating cell-centered data to
finite-volume faces. They are regular `MooseObject`s stored in the application's warehouse so they
can be declared once and shared by any kernel that needs them. Each method provides a small callable
handle that MOOSE kernels can cache, which keeps per-face evaluations free of virtual dispatch and
friendly to upcoming vectorized/GPU paths.

Interpolation methods are added in the `[FVInterpolationMethods]` block of an input file and
referenced through the `coeff_interp_method` parameter on kernels such as
[LinearFVDiffusion.md]. If the parameter is omitted the
kernel will keep evaluating the functor directly on the face, preserving the legacy behavior.

## Available methods

- Coefficient/face interpolation policies (e.g. geometric or harmonic averaging)
- Advected interpolation policies for advection terms:
  - [FVAdvectedMinmodWeightBased.md]
  - [FVAdvectedVanLeerWeightBased.md]

## Example input syntax

Declare the interpolation method in `[FVInterpolationMethods]`:

!listing test/tests/linearfvkernels/diffusion/diffusion-1d.i block=geom

Use it for a coefficient functor via
[!param](/LinearFVKernels/LinearFVDiffusion/coeff_interp_method):

!listing test/tests/linearfvkernels/diffusion/diffusion-1d.i block=diffusion replace=['diffusion_coeff = coeff_func','diffusion_coeff = coeff_pos_func\\n    coeff_interp_method = geom']
