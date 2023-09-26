# The Net Radiation Method System

The `GrayDiffuseRadiation` syntax invokes the
[RadiationTransferAction.md]. It simplifies setting up
gray, diffuse radiative exchange problems. It requires the user to provide boundaries,
emissivities on these boundaries, and the type of boundary: `ADIABATIC`, `FIXED_TEMPERATURE`, `VARIABLE_TEMPERATURE`; `VARIABLE_TEMPERATURE` are all boundaries that are not listed in the union of [!param](/GrayDiffuseRadiation/adiabatic_boundary) and [!param](/GrayDiffuseRadiation/fixed_temperature_boundary).  

The `GrayDiffuseRadiation` syntax allows to split the boundaries
into radiation patches using the [!param](/GrayDiffuseRadiation/n_patches) parameter. This allows the computation of more accurate
view factors between participating boundaries.

## Assumed geometry

The `GrayDiffuseRadiation` syntax assumes that radiation is exchanged in a cavity that is filled by a vacuum or a medium that is transparent to thermal radiation. The cavity must be enclosed by the boundaries specified in parameter [!param](/GrayDiffuseRadiation/boundary). If the cavity is not enclosed, the result of the computation may be incorrect without MOOSE issuing a warning or error.

By default, MOOSE's raytracing module is used for calculating view factors. The method is discussed in detail in
[RayTracingViewFactor.md]. The parameter [!param](/GrayDiffuseRadiation/view_factor_calculator) can be set to "analytical" to force
the use of [UnobstructedPlanarViewFactor.md]; at this point this is maintained as backward compatibility option and it may be removed in the future.

The [!param](/GrayDiffuseRadiation/ray_tracing_face_order) parameter is important because it controls the accuracy of the view factor computation.
The raytracing computation uses the `QGRID` quadrature available in libMesh which places quadrature points at uniform distances on the from and to boundary faces. Increasing the [!param](/GrayDiffuseRadiation/ray_tracing_face_order) increases the accuracy of the view factors. Note, that mesh refinement also enhances the accuracy of view factors, but at the expense of more elements being crossed during the raytracing procedure. Uniformly increasing the number of quadrature points using [!param](/GrayDiffuseRadiation/ray_tracing_face_order) is essentially equivalent to selectively refining the mesh on the faces in radiative transfer.

!syntax list /GrayDiffuseRadiation objects=True actions=False subsystems=False

!syntax list /GrayDiffuseRadiation objects=False actions=False subsystems=True

!syntax list /GrayDiffuseRadiation objects=False actions=True subsystems=False

## Example Input syntax

!listing modules/heat_conduction/test/tests/radiation_transfer_action/radiative_transfer_action.i
block=GrayDiffuseRadiation/cavity

!syntax parameters /GrayDiffuseRadiation
