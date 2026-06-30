# GrayDiffuseRadiation System

The `GrayDiffuseRadiation` syntax sets up
[gray, opaque, diffuse radiative heat transfer](modules/heat_transfer/index.md#gray_diffuse_radiative_exchange)
problems.

## Geometry and Boundary Types

This implementation assumes that radiation is exchanged in a cavity that is filled by a vacuum or a medium that is transparent to thermal radiation. The cavity must be enclosed by the boundaries specified in parameter [!param](/GrayDiffuseRadiation/boundary). If the cavity is not enclosed, the result of the computation may be incorrect without MOOSE issuing a warning or error.

The following boundary conditions have support:

| Boundary Type | Description | Parameters |
| :- | :- | :- |
| Adiabatic | $q_i = 0$ | [!param](/GrayDiffuseRadiation/boundary) and [!param](/GrayDiffuseRadiation/adiabatic_boundary) |
| Fixed temperature | $T_i$ known; provided by function via [!param](/GrayDiffuseRadiation/fixed_boundary_temperatures) | [!param](/GrayDiffuseRadiation/boundary) and [!param](/GrayDiffuseRadiation/fixed_temperature_boundary) |
| Variable temperature | $T_i$ unknown | [!param](/GrayDiffuseRadiation/boundary) only |
| Symmetry | symmetry about some plane | [!param](/GrayDiffuseRadiation/symmetry_boundary) only |

In summary:

- [!param](/GrayDiffuseRadiation/boundary) should contain all boundaries that participate in radiative exchange.
- Symmetry boundaries are the only type not participating in radiative exchange and thus do not appear in [!param](/GrayDiffuseRadiation/boundary).
- Any boundary in [!param](/GrayDiffuseRadiation/boundary) that is not in [!param](/GrayDiffuseRadiation/adiabatic_boundary) nor [!param](/GrayDiffuseRadiation/fixed_temperature_boundary) is a variable-temperature boundary.

## Boundary Patches

The surfaces in radiation exchange correspond to sidesets. Radiation quantities are averaged over each of these surfaces. The accuracy of the method can be improved by creating new sub-surfaces (called "patches") using the
[!param](/GrayDiffuseRadiation/n_patches) parameter.

## View Factors

By default, MOOSE's raytracing module is used for calculating view factors. The method is discussed in detail in
[RayTracingViewFactor.md]. The parameter [!param](/GrayDiffuseRadiation/view_factor_calculator) can be set to "analytical" to force
the use of [UnobstructedPlanarViewFactor.md]; at this point this is maintained as backward compatibility option and it may be removed in the future.

The [!param](/GrayDiffuseRadiation/ray_tracing_face_order) parameter is important because it controls the accuracy of the view factor computation.
The raytracing computation uses the `QGRID` quadrature available in libMesh which places quadrature points at uniform distances on the from and to boundary faces. Increasing the [!param](/GrayDiffuseRadiation/ray_tracing_face_order) increases the accuracy of the view factors. Note, that mesh refinement also enhances the accuracy of view factors, but at the expense of more elements being crossed during the raytracing procedure. Uniformly increasing the number of quadrature points using [!param](/GrayDiffuseRadiation/ray_tracing_face_order) is essentially equivalent to selectively refining the mesh on the faces in radiative transfer.

## Implementation

The following tasks are performed by this syntax/action:

- New sidesets are created with [PatchSidesetGenerator.md], corresponding to the numbers of
  patches specified by [!param](/GrayDiffuseRadiation/n_patches).
- A [GrayLambertSurfaceRadiationBase.md] object is created.
- [GrayLambertNeumannBC.md] objects are created for boundaries requiring BC.
- A [UnobstructedPlanarViewFactor.md] ([!param](/GrayDiffuseRadiation/view_factor_calculator) is `analytical`)
  or [RayTracingViewFactor.md] ([!param](/GrayDiffuseRadiation/view_factor_calculator) is `ray_tracing`)
  object is created to compute view factors.
- A [ViewFactorRayStudy.md] is created if [!param](/GrayDiffuseRadiation/view_factor_calculator) is `ray_tracing`.
- [ViewFactorRayBC.md] and [ReflectRayBC.md] objects are created if [!param](/GrayDiffuseRadiation/view_factor_calculator) is `ray_tracing`.

## Associated Objects

The following objects are useful for querying a [GrayLambertSurfaceRadiationBase.md] object:

- [GrayLambertSurfaceRadiationPP.md] is a post-processor that retrieves the radiosity, heat flux, or temperature
  for a boundary.
- [SurfaceRadiationVectorPostprocessor.md] is a vector post-processor that retrieves one or more of the following
  for all surfaces: emissivity, radiosity, temperature, and heat flux.

The following objects are useful for querying a [ViewFactorBase.md] object:

- [ViewFactorPP.md] is a post-processor that can retrieve a view factor.
- [ViewFactorVectorPostprocessor.md] is a vector post-processor that retrieves the view factors
  between all boundaries.

!syntax list /GrayDiffuseRadiation objects=True actions=False subsystems=False

!syntax list /GrayDiffuseRadiation objects=False actions=False subsystems=True

!syntax list /GrayDiffuseRadiation objects=False actions=True subsystems=False

## Example Input syntax

!listing modules/heat_transfer/test/tests/radiation_transfer_action/radiative_transfer_action.i
block=GrayDiffuseRadiation/cavity

!syntax parameters /GrayDiffuseRadiation
