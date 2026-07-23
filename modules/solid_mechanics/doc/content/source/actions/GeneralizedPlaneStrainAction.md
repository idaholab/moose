# GeneralizedPlaneStrainAction

!syntax description /Physics/SolidMechanics/GeneralizedPlaneStrain/GeneralizedPlaneStrainAction

## Description

This action simplifies the input syntax for creating a generalized plane strain
model. It creates the objects associated with the scalar out-of-plane strain
variable and the out-of-plane equilibrium equation. Use
[!param](/Physics/SolidMechanics/GeneralizedPlaneStrain/GeneralizedPlaneStrainAction/use_automatic_differentiation)
to select the AD construction path.

## Constructed Objects

In legacy mode, the action creates the same UserObject, ScalarKernel, and
off-diagonal Kernel objects used by existing generalized plane strain inputs.

!table id=gps_legacy_action_objects caption=Legacy objects created by `GeneralizedPlaneStrainAction`
| Object | Purpose |
| --- | --- |
| [GeneralizedPlaneStrainOffDiag.md] | Couples the in-plane displacement variables and the scalar out-of-plane strain variable in the off-diagonal Jacobian. |
| [GeneralizedPlaneStrain.md] | Assembles the scalar out-of-plane equilibrium residual. |
| [GeneralizedPlaneStrainUserObject.md] | Computes the residual and diagonal Jacobian data used by the scalar kernel. |

In AD mode, the action creates exactly one [ADGeneralizedPlaneStrain.md] object.
The legacy UserObject, ScalarKernel, and off-diagonal Kernel objects are not
created because `ADKernelScalarBase` assembles the scalar residual and its
couplings through automatic differentiation. When
[!param](/Physics/SolidMechanics/GeneralizedPlaneStrain/GeneralizedPlaneStrainAction/use_displaced_mesh)
is `true`, the AD kernel uses displaced-mesh quadrature weights for that scalar
assembly.

!table id=gps_ad_action_objects caption=AD objects created by `GeneralizedPlaneStrainAction`
| Object | Purpose |
| --- | --- |
| [ADGeneralizedPlaneStrain.md] | Assembles the scalar out-of-plane equilibrium residual and AD couplings. |

The action also creates the scalar out-of-plane strain variable when it is
missing. The automatically created scalar variable is a `FIRST` order nonlinear
scalar variable in nonlinear system `nl0`. If the named scalar variable already
exists in `nl0`, the action reuses it. A field variable or a scalar variable in a
different nonlinear system with the same name is rejected.

## Out-of-Plane Pressure

The residual assembled for the scalar out-of-plane strain is
\begin{equation}
  R_s = \int_A \left( \sigma_{oo} + p_o \right) dA = 0,
\end{equation}
where $p_o$ is the out-of-plane pressure. Positive pressure is applied toward the
body. The pressure may be supplied with
[!param](/Physics/SolidMechanics/GeneralizedPlaneStrain/GeneralizedPlaneStrainAction/out_of_plane_pressure_function)
or with
[!param](/Physics/SolidMechanics/GeneralizedPlaneStrain/GeneralizedPlaneStrainAction/out_of_plane_pressure_material).
In AD mode, the pressure material is read as a regular `Real` material property,
so it does not add derivatives to the scalar equation.

## Generalized Plane Strain and Reference Residual

Generalized plane strain problems may use `ReferenceResidualProblem`. In this case, a reference scalar variable is needed to correspond with the scalar strain variable.

!listing modules/combined/test/tests/generalized_plane_strain_tm_contact/out_of_plane_pressure.i block=Problem

The reference scalar variable is set using the `AuxScalarKernel` [Generalized Plane Strain Reference Residual](/GeneralizedPlaneStrainReferenceResidual.md) using the [Generalized Plane Strain UserObject](/GeneralizedPlaneStrainUserObject.md).

!listing modules/combined/test/tests/generalized_plane_strain_tm_contact/out_of_plane_pressure.i block=AuxScalarKernels

## Example Input Syntax

The following test-spec block shows a modern QuasiStatic generalized plane
strain input run in AD mode.

!listing modules/solid_mechanics/test/tests/generalized_plane_strain/tests start=[generalized_plane_strain_small_ad] end=[] include-end=true

The following input shows a standalone
`[Physics/SolidMechanics/GeneralizedPlaneStrain]` action block. The action
creates the missing scalar out-of-plane strain variable in AD mode.

!listing modules/solid_mechanics/test/tests/generalized_plane_strain/generalized_plane_strain_auto_scalar.i block=Physics/SolidMechanics/GeneralizedPlaneStrain

!syntax parameters /Physics/SolidMechanics/GeneralizedPlaneStrain/GeneralizedPlaneStrainAction
