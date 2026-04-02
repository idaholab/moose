# FVInterpolationMethods System

`FVInterpolationMethod` objects define how cell-centered quantities are interpolated to finite-volume
faces. Methods are declared in the `[FVInterpolationMethods]` block and referenced by name from
kernels that support them.

These methods are commonly used by linear FV kernels. For example:

- [LinearFVDiffusion.md] can use a face interpolation method for coefficients via
  [!param](/LinearFVKernels/LinearFVDiffusion/coeff_interp_method).
- [LinearFVAdvection.md] requires an advected interpolation method name via
  [!param](/LinearFVKernels/LinearFVAdvection/advected_interp_method_name).

For a general overview of FV interpolation and design choices, see
[/linear_fv_design.md].

## Design

`FVInterpolationMethod` derived classes override the operations they support:

- `interpolate(...)` for coefficient/face interpolation
- `advectedInterpolate(...)` for advected interpolation (matrix weights + right-hand-side face term)
- optionally `advectedInterpolationNeedsGradients()` and `gradientLimiter()` when the method needs
  adjacent-cell gradients

We enable guard rails to ensure that advected interpolation methods are not used out of context
and interpolation methods not supporting advection schemes are not used in that context either.
For examples on how to enforce this, see:

- `LinearFVDiffusion` only accepts methods with `supportsFaceInterpolation() == true`.
- `LinearFVAdvection` only accepts methods with `supportsAdvectedInterpolation() == true`.

Selecting an incompatible interpolation method causes a runtime error during kernel setup.

## FVInterpolationMethods block

Declare methods in the `[FVInterpolationMethods]` block, for example a geometric average used to
interpolate diffusion coefficients:

!listing test/tests/linearfvkernels/diffusion/diffusion-1d.i
         block=geom

Use it in a kernel via the interpolation method name in a dedicated parameter:

!listing test/tests/linearfvkernels/diffusion/diffusion-1d.i
         block=diffusion

For advection, declare an advected interpolation method (for example, upwind):

!listing test/tests/linearfvkernels/advection/advection-1d.i
         block=upwind

Use it in a linear FV advection kernel:

!listing test/tests/linearfvkernels/advection/advection-1d.i
         block=advection

See the individual method pages listed below for details.

!syntax list /FVInterpolationMethods objects=True actions=False subsystems=False
