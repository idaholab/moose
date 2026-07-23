# ADGeneralizedPlaneStrain

!syntax description /Kernels/ADGeneralizedPlaneStrain

## Description

`ADGeneralizedPlaneStrain` assembles the scalar out-of-plane equilibrium equation
for an automatic-differentiation generalized plane strain model. It contributes no
field residual to its anchor displacement variable; instead, `ADKernelScalarBase`
owns the scalar residual assembly and uses the selected block's quadrature,
`JxW`, and coordinate weights to assemble
\begin{equation}
  R_s = \int_A \left( \sigma_{oo} + p_o \right) dA = 0,
\end{equation}
where $s$ is the scalar out-of-plane strain variable, $\sigma_{oo}$ is the stress
component in the out-of-plane direction, and $p_o$ is the prescribed
out-of-plane pressure. Positive out-of-plane pressure is applied toward the body,
matching the pressure convention used by the generalized plane strain action.

The out-of-plane direction is selected with
[!param](/Kernels/ADGeneralizedPlaneStrain/out_of_plane_direction) in Cartesian
coordinates. The `x`, `y`, and `z` options map to stress components
$\sigma_{xx}$, $\sigma_{yy}$, and $\sigma_{zz}$, respectively. In an RZ
axisymmetric model the out-of-plane direction is the axial `y` direction, and the
scalar residual is integrated with the axisymmetric coordinate weighting supplied
by `ADKernelScalarBase`. When
[!param](/Kernels/ADGeneralizedPlaneStrain/use_displaced_mesh) is `true`, those
weights are evaluated on the displaced mesh.

This kernel requires an automatic-differentiation stress material property named
`stress`, or `<base_name>_stress` when
[!param](/Kernels/ADGeneralizedPlaneStrain/base_name) is supplied. The
[!param](/Kernels/ADGeneralizedPlaneStrain/out_of_plane_pressure_material)
parameter accepts a regular `Real` material property, so the pressure material
does not contribute derivatives to the scalar equation. Use a function through
[!param](/Kernels/ADGeneralizedPlaneStrain/out_of_plane_pressure_function) when
the pressure only needs to vary in time or space.

## Example Input File Syntax

The action normally creates this kernel. The test-spec block below runs the
standard small-strain generalized plane strain input in AD mode by switching the
mechanics objects and output objects to their AD variants.

!listing modules/solid_mechanics/test/tests/generalized_plane_strain/tests start=[generalized_plane_strain_small_ad] end=[] include-end=true

!syntax parameters /Kernels/ADGeneralizedPlaneStrain

!syntax inputs /Kernels/ADGeneralizedPlaneStrain

!syntax children /Kernels/ADGeneralizedPlaneStrain
