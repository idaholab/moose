# Generalized Plane Strain

## Description

Generalized plane strain extends plane strain by allowing a nonzero constant
strain in the out-of-plane direction [!citep](Adams1967gps). Other formulations
add two rotational degrees of freedom [!citep](Abaqus2014gps) or include the
anticlastic problem associated with out-of-plane shear [!citep](Adams1984gps, Li1999gps).

The [solid mechanics module](solid_mechanics/index.md) implements the form with
one extra degree of freedom representing the direct out-of-plane strain
[!citep](Li2005gps).

## Formulation

For the usual $x$-$y$ model plane, the $z$ axis is the out-of-plane direction.
Under generalized plane strain conditions, the in-plane stresses are not
functions of $z$, and the direct out-of-plane strain is represented by a scalar
variable. The model is solved on a 2D domain.

The formulation also works when the model plane is $x$-$z$ or $y$-$z$. In those
cases the out-of-plane direction is the $y$ axis or $x$ axis, respectively. In
RZ axisymmetry, the out-of-plane generalized plane strain direction is the axial
$y$ direction.

## $x$-$y$ plane generalized plane strain problem

### In-plane equilibrium equations

The kinematical equations of the generalized plane strain problem are identical to those for the plane stress or strain problems, given as
\begin{equation}
\label{eqn:xy_kinematic_equations}
\left\{\begin{matrix}
\epsilon_{xx} \\
\epsilon_{yy} \\
\gamma_{xy}
\end{matrix}\right\}
=
\left\{\begin{matrix}
\frac{\partial u}{\partial x} \\
\frac{\partial v}{\partial y} \\
\frac{\partial v}{\partial x} + \frac{\partial u}{\partial y}
\end{matrix}\right\}
\end{equation}

The equilibrium equations for the generalized plane strain problem in the $x$-$y$ plane are given as
\begin{equation}
\begin{matrix}
\frac{\partial \sigma_{xx}}{\partial x} + \frac{\partial \sigma_{xy}}{\partial y} + \bar{f}_{x} = 0 \\
\frac{\partial \sigma_{xy}}{\partial x} + \frac{\partial \sigma_{yy}}{\partial y} + \bar{f}_{y} = 0
\end{matrix}
\end{equation}
in $A$ where $\left\{ \bar{f} \right\}$ are the body forces.

The constitutive equations in terms of stress-strain relationship are given as
\begin{equation}
\left\{ \begin{matrix}
\sigma_{xx} \\
\sigma_{yy} \\
\sigma_{zz} \\
\tau_{xy}
\end{matrix} \right\}
=
\left[ \begin{matrix}
C_{11} & C_{12} & C_{13} & 0 \\
C_{12} & C_{22} & C_{23} & 0 \\
C_{13} & C_{23} & C_{33} & 0 \\
0 & 0 & 0 & C_{66}
\end{matrix} \right]
\left( \left\{ \begin{matrix}
\epsilon_{xx} \\
\epsilon_{yy} \\
\epsilon_{zz} \\
\gamma_{xy}
\end{matrix} \right\}
-
\left\{ \begin{matrix}
\alpha_{x} \\
\alpha_{y} \\
\alpha_{z} \\
0
\end{matrix} \right\} \Delta T \right)
\end{equation}
where $C_{ij}$ are material's stiffness coefficients using Voigt notation, $\alpha_{x}$, $\alpha_{y}$ and $\alpha_{z}$ are the material's thermal expansion coefficients and $\Delta T$ is the change in temperature.

### Out-of-plane equilibrium equation

A further condition is required associated with the out-of-plane direction. If a constant strain is prescribed as the deformation compatibility condition in the $z$-direction
\begin{equation}
\epsilon_{zz} = \bar{\epsilon}_{zz}
\end{equation}
Alternatively, a force as the stress resultant $\bar{N}_{z}$ in the $z$-direction can be prescribed. The condition is the equilibrium condition in $z$-direction, given as
\begin{equation}
\label{eqn:xy_equilibrium}
N_{z} = \int_{A}{\sigma_{zz}dA} = \bar{N}_{z}
\end{equation}
The stress resultant $N_{z}$ conjugates with constant strain $\epsilon_{zz}$.

The formulation above shares the in-plane mechanics of a conventional plane
strain problem and adds the scalar degree of freedom for the out-of-plane
direction.

When an out-of-plane pressure $p_o$ is prescribed, the scalar residual is written
as
\begin{equation}
  R_s = \int_A \left( \sigma_{oo} + p_o \right) dA = 0,
\end{equation}
where $o$ is the selected out-of-plane direction. Positive out-of-plane pressure
is applied toward the body. In Cartesian coordinates, $o=x$, $o=y$, and $o=z$
use $\sigma_{xx}$, $\sigma_{yy}$, and $\sigma_{zz}$, respectively. In RZ
axisymmetry, $o=y$, and the scalar residual is integrated with the RZ coordinate
weighting.

### Implementation

The out-of-plane strain is a scalar variable in the standard system of equations
for a mechanics problem, where $\boldsymbol{u}_x$ and $\boldsymbol{u}_y$
represent the displacement vectors in the $x$ and $y$ directions, and
$\boldsymbol{f}_x$ and $\boldsymbol{f}_y$ represent the corresponding reaction
forces. For a two-dimensional model in the $x$-$y$ plane, the partitioned
linearized system can be written as follows:

\begin{equation}
\left[
\begin{array}{cc|c}
\boldsymbol{K}_{xx} & \boldsymbol{K}_{xy} & \boldsymbol{K}_{xz} \\
\boldsymbol{K}_{yx} & \boldsymbol{K}_{yy} & \boldsymbol{K}_{yz} \\ \hline
\boldsymbol{K}_{zx} & \boldsymbol{K}_{zy} & K_{zz}
\end{array}
\right]
\left\{
\begin{array}{c}
\boldsymbol{u}_x \\
\boldsymbol{u}_y \\ \hline
\epsilon_{zz}
\end{array}
\right\}
=
\left\{
\begin{array}{c}
\boldsymbol{f}_x \\
\boldsymbol{f}_y \\ \hline
N_{z}
\end{array}
\right\}
\end{equation}

The off-diagonal entries are nonzero, but not shown here.

## $x$-$z$ and $y$-$z$ plane generalized plane strain problem

The generalized plane strain formulation can also be used if the two-dimensional model is represented in the $x$-$z$ or $y$-$z$ planes, rather than the $x$-$y$ plane, as is typically the case. If the model lies in those other planes, the calculation of the strain tensor is modified to take into account the fact that the model is in a different plane. Also, the scalar variable used to represent the out-of-plane strain in the generalized plane strain formulation is in a different direction. All other aspects of the formulation are identical to the $x$-$y$ plane case.

For the case when the model lies in the $x$-$z$ plane, the small-strain kinematic equations for the strain calculation that are equivalent to [eqn:xy_kinematic_equations] for the $x$-$y$ plane are expressed as:
\begin{equation}
\left\{\begin{matrix}
\epsilon_{xx} \\
\epsilon_{zz} \\
\gamma_{xz}
\end{matrix}\right\}
=
\left\{\begin{matrix}
\frac{\partial u}{\partial x} \\
\frac{\partial w}{\partial z} \\
\frac{\partial w}{\partial x} + \frac{\partial u}{\partial z}
\end{matrix}\right\}
\end{equation}
The generalized plane strain equilibrium equation equivalent to [eqn:xy_equilibrium] is expressed as:
\begin{equation}
N_{y} = \int_{A}{\sigma_{yy}dA} = \bar{N}_{y}
\end{equation}
The same pattern is followed for the $y$-$z$ plane case.

## MOOSE Objects

Objects available for generalized plane strain:

- [Stress Divergence Kernel](/StressDivergence.md): in-plane equilibrium equation

- [Stress Models](solid_mechanics/Stresses.md): full stress tensor calculation

Objects specific for generalized plane strain:

- [Generalized Plane Strain ScalarKernel](/GeneralizedPlaneStrain.md): out-of-plane equilibrium condition

- [Generalized Plane Strain UserObject](/GeneralizedPlaneStrainUserObject.md): residual and diagonal Jacobian calculation for scalar out-of-plane strain variable

- [Generalized Plane Strain Off-diagonal Kernel](/GeneralizedPlaneStrainOffDiag.md): in-plane displacement variables and scalar out-of-plane strain coupling

- [Strain Calculations](solid_mechanics/Strains.md): in-plane strain calculation and formation of full strain tensor considering the scalar out-of-plane strain

Objects specific to the AD generalized plane strain path:

- [ADGeneralizedPlaneStrain.md]: scalar out-of-plane equilibrium condition and
  AD coupling terms

!table id=gps_legacy_objects caption=Legacy generalized plane strain objects created by the action
| Object | Role |
| --- | --- |
| [GeneralizedPlaneStrainOffDiag.md] | Off-diagonal coupling between in-plane displacement variables and the scalar out-of-plane strain. |
| [GeneralizedPlaneStrain.md] | Scalar out-of-plane equilibrium residual. |
| [GeneralizedPlaneStrainUserObject.md] | Residual and diagonal Jacobian data for the scalar kernel. |

!table id=gps_ad_objects caption=AD generalized plane strain objects created by the action
| Object | Role |
| --- | --- |
| [ADGeneralizedPlaneStrain.md] | Scalar out-of-plane equilibrium residual and AD coupling terms. |

The AD path requires AD stress materials. [ADGeneralizedPlaneStrain.md] owns the
scalar residual assembly through `ADKernelScalarBase`, including displaced-mesh
quadrature weights when requested. The out-of-plane pressure material used by
[ADGeneralizedPlaneStrain.md] is a regular `Real` material property, so it is
treated as derivative-free in the scalar equation.

## How to Use Generalized Plane Strain

The [GeneralizedPlaneStrainAction](/GeneralizedPlaneStrainAction.md) can be used to set up a generalized plane strain model. The [QuasiStaticSolidMechanicsPhysics](/QuasiStaticSolidMechanicsPhysics.md) which considers the [GeneralizedPlaneStrainAction](/GeneralizedPlaneStrainAction.md) as Meta-Action can also be used.

The following test-spec block shows a modern QuasiStatic small-strain input run
in AD mode.

!listing modules/solid_mechanics/test/tests/generalized_plane_strain/tests start=[generalized_plane_strain_small_ad] end=[] include-end=true

The following standalone action input uses AD mode and lets the action create the
missing scalar out-of-plane strain variable.

!listing modules/solid_mechanics/test/tests/generalized_plane_strain/generalized_plane_strain_auto_scalar.i block=Physics/SolidMechanics/GeneralizedPlaneStrain

!bibtex bibliography
