# Generalized Plane Strain Action System

!syntax description /Physics/SolidMechanics/GeneralizedPlaneStrain/GeneralizedPlaneStrainAction

## Description

This action sets up a generalized plane strain model. A detailed description of
the formulation is available on the [generalized plane strain](solid_mechanics/generalized_plane_strain.md)
page.

!alert! warning title=For 2D and 1D Simulations
`GeneralizedPlaneStrainAction` supports 1D axisymmetric or 2D generalized plane
strain cases. For 1D axisymmetric and 2D Cartesian cases in the x-y plane, the
number of displacement variables must be one or two, respectively.

For 2D generalized plane strain cases in the x-z or y-z planes, the number of
displacement variables must be three.
!alert-end!

## Constructed MooseObjects

The `GeneralizedPlaneStrain` Action constructs the objects and scalar
out-of-plane strain variable required for a generalized plane strain simulation.
When the scalar variable named by
[!param](/Physics/SolidMechanics/GeneralizedPlaneStrain/GeneralizedPlaneStrainAction/scalar_out_of_plane_strain)
does not exist, the action creates a `FIRST` order nonlinear scalar variable in
nonlinear system `nl0`. If that scalar variable already exists in `nl0`, the
action reuses it.

!table id=gps_legacy_action_table caption=Legacy objects created by `GeneralizedPlaneStrainAction`
| Object | Purpose |
| --- | --- |
| [GeneralizedPlaneStrainOffDiag.md] | Couples in-plane displacement variables and the scalar out-of-plane strain variable in the off-diagonal Jacobian. |
| [GeneralizedPlaneStrain.md] | Assembles the scalar out-of-plane equilibrium residual. |
| [GeneralizedPlaneStrainUserObject.md] | Computes residual and diagonal Jacobian data for the scalar kernel. |

When
[!param](/Physics/SolidMechanics/GeneralizedPlaneStrain/GeneralizedPlaneStrainAction/use_automatic_differentiation)
is `true`, the action creates exactly one AD object and skips the legacy
UserObject, ScalarKernel, and off-diagonal Kernel objects.
`ADKernelScalarBase` owns the scalar residual assembly in this mode; when
[!param](/Physics/SolidMechanics/GeneralizedPlaneStrain/GeneralizedPlaneStrainAction/use_displaced_mesh)
is `true`, the AD kernel uses displaced-mesh quadrature weights for the scalar
equation.

!table id=gps_ad_action_table caption=AD objects created by `GeneralizedPlaneStrainAction`
| Object | Purpose |
| --- | --- |
| [ADGeneralizedPlaneStrain.md] | Assembles the scalar out-of-plane equilibrium residual and AD couplings. |

The scalar residual is
\begin{equation}
  R_s = \int_A \left( \sigma_{oo} + p_o \right) dA = 0,
\end{equation}
where $p_o$ is positive when applied toward the body. In Cartesian coordinates,
the out-of-plane `x`, `y`, and `z` directions use $\sigma_{xx}$,
$\sigma_{yy}$, and $\sigma_{zz}$, respectively. In RZ axisymmetry, the
out-of-plane direction is the axial `y` direction and the residual uses
axisymmetric coordinate weighting.

## Example Input Syntax

### Subblocks

The subblocks of the GeneralizedPlaneStrain action trigger object construction.
If a generalized plane strain model is applied to the whole simulation domain,
use a single subblock.

!listing modules/solid_mechanics/test/tests/generalized_plane_strain/out_of_plane_pressure.i block=Physics/SolidMechanics/GeneralizedPlaneStrain

If different mesh subdomains use different generalized plane strain models, use
multiple subblocks with subdomain restrictions.

!listing modules/solid_mechanics/test/tests/generalized_plane_strain/generalized_plane_strain_squares.i block=Physics/SolidMechanics/GeneralizedPlaneStrain

An example of using generalized plane strain through the Solid Mechanics
QuasiStatic physics block with an
[!param](/Physics/SolidMechanics/GeneralizedPlaneStrain/GeneralizedPlaneStrainAction/out_of_plane_direction)
different from the default $z$ direction is given by:

!listing modules/solid_mechanics/test/tests/2D_different_planes/gps_xz.i block=Physics/SolidMechanics/QuasiStatic/generalized_plane_strain

Parameters supplied at the `[Physics/SolidMechanics/GeneralizedPlaneStrain]` level act as
defaults for the QuasiStatic Physics subblocks.

The following test-spec block shows the standard small-strain generalized plane
strain input run with automatic differentiation.

!listing modules/solid_mechanics/test/tests/generalized_plane_strain/tests start=[generalized_plane_strain_small_ad] end=[] include-end=true

The following standalone action input lets the action create the missing scalar
out-of-plane strain variable in AD mode.

!listing modules/solid_mechanics/test/tests/generalized_plane_strain/generalized_plane_strain_auto_scalar.i block=Physics/SolidMechanics/GeneralizedPlaneStrain

!syntax parameters /Physics/SolidMechanics/GeneralizedPlaneStrain/GeneralizedPlaneStrainAction


## Associated Actions

!syntax list /Physics/SolidMechanics/GeneralizedPlaneStrain objects=True actions=False subsystems=False

!syntax list /Physics/SolidMechanics/GeneralizedPlaneStrain objects=False actions=False subsystems=True

!syntax list /Physics/SolidMechanics/GeneralizedPlaneStrain objects=False actions=True subsystems=False
