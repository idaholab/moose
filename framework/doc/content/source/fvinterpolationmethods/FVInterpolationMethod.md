# FVInterpolationMethod

## Description

`FVInterpolationMethod` objects define interpolate cell-centered data to
finite-volume faces. They are regular `MooseObject`s stored in the application's warehouse so they
can be declared once and shared by any kernel that needs them.

Interpolation methods are added in the `[FVInterpolationMethods]` block of an input file and can be
referenced through the parameters on kernels such as
[LinearFVDiffusion.md]. If the parameter is omitted the
kernel will keep evaluating the functor directly on the face, preserving the legacy behavior.

## Example input syntax

Declare the interpolation method in `[FVInterpolationMethods]`:

!listing test/tests/linearfvkernels/diffusion/diffusion-1d.i block=geom

Use it for a coefficient functor via
[!param](/LinearFVKernels/LinearFVDiffusion/coeff_interp_method):

!listing test/tests/linearfvkernels/diffusion/diffusion-1d.i block=diffusion replace=['diffusion_coeff = coeff_func','diffusion_coeff = coeff_pos_func\\n    coeff_interp_method = geom']
