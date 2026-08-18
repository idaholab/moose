# ADGeneralizedPlaneStrain

!syntax description /Kernels/ADGeneralizedPlaneStrain

## Description

`ADGeneralizedPlaneStrain` assembles the scalar out-of-plane equilibrium equation
for an automatic-differentiation generalized plane strain model. It contributes no
field residual to the displacement variables; instead, it performs area integrals
over finite elements to compute the contribution to the following scalar residual $R_s$:
\begin{equation}
  R_s = \int_A \left( \sigma_{oo} + p_o \right) dA = 0,
\end{equation}
where $s$ is the scalar out-of-plane strain variable, $\sigma_{oo}$ is the stress
component in the out-of-plane direction, and $p_o$ is a prescribed
out-of-plane pressure which by convention is applied toward the body.

This kernel can be used with two-dimensional models defined in the x-y, x-z, or y-z
planes. The out-of-plane direction can be specified with
[!param](/Kernels/ADGeneralizedPlaneStrain/out_of_plane_direction), and should be
the direction out of the model plane. The `x`, `y`, and `z` options map to stress components
$\sigma_{xx}$, $\sigma_{yy}$, and $\sigma_{zz}$, respectively. In a one-dimensional
axisymmetric model the out-of-plane direction is the axial `y` direction. When
[!param](/Kernels/ADGeneralizedPlaneStrain/use_displaced_mesh) is `true`, the area integral
is evaluated on the displaced mesh.

This kernel requires an automatic-differentiation stress material property named
`stress`, or `<base_name>_stress` when
[!param](/Kernels/ADGeneralizedPlaneStrain/base_name) is supplied. The
out-of-plane pressure can optionally be prescribed through either a material property
or a function. The [!param](/Kernels/ADGeneralizedPlaneStrain/out_of_plane_pressure_material)
parameter accepts a regular `Real` material property, so the pressure material
does not contribute derivatives to the scalar equation.
[!param](/Kernels/ADGeneralizedPlaneStrain/out_of_plane_pressure_function) can be used
to prescribe out-of-plane pressure that varies only in time or space. If both are
given, the two contributions are summed. The total prescribed pressure is scaled
by [!param](/Kernels/ADGeneralizedPlaneStrain/pressure_factor).

## Example Input File Syntax

The [GeneralizedPlaneStrainAction](/GeneralizedPlaneStrainAction.md) normally
creates this kernel. The following action block causes the action to create an
`ADGeneralizedPlaneStrain` kernel:

!listing modules/solid_mechanics/test/tests/generalized_plane_strain/generalized_plane_strain_auto_scalar.i block=Physics/SolidMechanics/QuasiStatic

!syntax parameters /Kernels/ADGeneralizedPlaneStrain

!syntax inputs /Kernels/ADGeneralizedPlaneStrain

!syntax children /Kernels/ADGeneralizedPlaneStrain
