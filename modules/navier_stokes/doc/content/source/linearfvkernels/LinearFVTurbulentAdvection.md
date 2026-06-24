# LinearFVTurbulentAdvection

This object adds a $\nabla \cdot \vec u \phi$ term for an arbitrary scalar field
$\phi$, where $\phi$ corresponds to the variable that this kernel acts
on. It uses the linear finite volume discretization.
The [!param](/LinearFVKernels/LinearFVTurbulentAdvection/variable) can be of type `MooseLinearVariableFVReal`.

The particularity of this kernel is that it allows us to skip computing advection
for near-wall elements.
For any element that is in contact with a boundary identified
in the [!param](/LinearFVKernels/LinearFVTurbulentAdvection/walls) list,
advection will be skipped for that element over all faces.

## Selecting the interpolation method

The [!param](/LinearFVKernels/LinearFVTurbulentAdvection/advected_interp_method_name) parameter is
the name of an interpolation method in the `[FVInterpolationMethods]` block. For example, to use
upwind interpolation, add an [FVAdvectedUpwind.md] method and set
[!param](/LinearFVKernels/LinearFVTurbulentAdvection/advected_interp_method_name) to that method
name:

!listing modules/navier_stokes/test/tests/finite_volume/ins/turbulence/channel/linear-segregated/channel_ERCOFTAC.i block=FVInterpolationMethods

!listing modules/navier_stokes/test/tests/finite_volume/ins/turbulence/channel/linear-segregated/channel_ERCOFTAC.i block=LinearFVKernels/TKE_advection

When using [WCNSLinearFVTurbulencePhysics.md], the
[!param](/Physics/NavierStokes/TurbulenceSegregated/WCNSLinearFVTurbulencePhysics/tke_advection_interpolation)
and [!param](/Physics/NavierStokes/TurbulenceSegregated/WCNSLinearFVTurbulencePhysics/tked_advection_interpolation)
shortcuts can be set directly, for example `tke_advection_interpolation = upwind` and
`tked_advection_interpolation = upwind`. No `[FVInterpolationMethods]` block is needed for the
Physics shortcuts.

!syntax parameters /LinearFVKernels/LinearFVTurbulentAdvection

!syntax inputs /LinearFVKernels/LinearFVTurbulentAdvection

!syntax children /LinearFVKernels/LinearFVTurbulentAdvection
