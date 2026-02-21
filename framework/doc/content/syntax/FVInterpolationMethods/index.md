# FVInterpolationMethods System

`FVInterpolationMethod` objects define how cell-centered quantities are interpolated to finite-volume
faces. Methods are declared in the `[FVInterpolationMethods]` block and referenced by name from
kernels that support them.

These methods are commonly used by linear FV kernels. For example:

- [LinearFVDiffusion](syntax/LinearFVKernels/index.md) can use a face interpolation method for
  coefficients via [!param](/LinearFVKernels/LinearFVDiffusion/coeff_interp_method).
- [LinearFVAdvection](syntax/LinearFVKernels/index.md) requires an advected interpolation method
  name via [!param](/LinearFVKernels/LinearFVAdvection/advected_interp_method_name).

For a general overview of FV interpolation and design choices, see [/fv_design.md] and
[/linear_fv_design.md].

## Design

FVInterpolationMethods build small, trivially-copyable callable handles
for face interpolation. Kernels cache these handles and invoke them inside face loops
This system is designed to enable vectorization. Each handle carries a small `DeviceData` object
(in a fixed-size buffer) plus interpolation-specific information such as the blending ratios,
limiting options, etc. This design keeps evaluation fast while still allowing interpolation
methods to be tunable from the input file.

When adding a new method, implement the structures and functions you intend to expose:

- A `DeviceData` struct that is trivially copyable and fits within the handle storage. This structure
  will be responsible to inform the interpolation method of the specific parameters needed. Essentially, it hands over the user-specified parameter in the input file.
- `interpolate(...)`
  if you want a coefficient/face interpolation policy. This method will not incorporate
  advection-related information.
- `advectedInterpolate`
  if you want advected interpolation weights and right hand side contributions from a face value.
- `advectedInterpolateValue` if the user also wants a direct face-value from an advected interpolation.

## FVInterpolationMethods block

Declare methods in the `[FVInterpolationMethods]` block, for example a geometric average used to
interpolate diffusion coefficients:

!listing test/tests/linearfvkernels/diffusion/diffusion-1d.i
         block=geom

Use it in a kernel via a method name parameter:

!listing test/tests/linearfvkernels/diffusion/diffusion-1d.i
         block=diffusion

For advection, declare an advected interpolation method (e.g., upwind):

!listing test/tests/linearfvkernels/advection/advection-1d.i
         block=upwind

Use it in a linear FV advection kernel:

!listing test/tests/linearfvkernels/advection/advection-1d.i
         block=advection

## Available methods

See the individual method pages for details:

- [FVGeometricAverage.md]
- [FVHarmonicAverage.md]
- [FVAdvectedUpwind.md]
- [FVAdvectedMinmodWeightBased.md]
- [FVAdvectedVanLeerWeightBased.md]
- [FVAdvectedVenkatakrishnanDeferredCorrection.md]

!syntax list /FVInterpolationMethods objects=True actions=False subsystems=False
