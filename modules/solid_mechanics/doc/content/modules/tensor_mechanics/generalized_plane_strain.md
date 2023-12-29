# Generalized Plane Strain

## Description

The generalized plane strain problem has found use in many important applications. Many different presentations can be found in the literature. The simplest one is an extension of the plane strain problem by allowing a non-vanishing constant direction strain in the out-of-plane direction [!citep](Adams1967gps). It has been further generalized by allowing two rotations degrees of freedom [!citep](Abaqus2014gps). An even more generalized form includes the anticlastic problem associated with the out-of-plane shear [!citep](Adams1984gps, Li1999gps).

In the [tensor mechanics module](tensor_mechanics/index.md), the simplest form was implemented, i.e. an extra degree of freedom representing the out-of-plane direct strain is included in addition to the conventional plane strain problem [!citep](Li2005gps).

## Formulation

For description, we introduce the coordinate system so that the $x$-$y$ plane is positioned transverse to the perpendicular out-of-plane direction and the $z$-axis runs in this out-of-plane direction. Under the simplest generalized plane strain conditions, the in-plane stresses are not functions of $z$, implying that the in-plane strains are not functions of $z$ from the constitutive relationship for linearly elastic and homogeneous materials. +The generalized plane strain problem is 2D in presentation, defined in a 2D domain+.

The generalized plane strain formulation also works when the coordinate system is such that the $x$-$z$ or $y$-$z$ planes are positioned transverse to the perpendicular out-of-plane direction.  In these cases the $y$-axis or $x$-axis runs in the out-of-plane direction, respectively.

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
Alternatively, a force as the stress resultant $\bar{N}_{z}$ in the $z$-direction can be prescribed, the condition is the equilibrium condition in $z$-direction, given as
\begin{equation}
\label{eqn:xy_equilibrium}
N_{z} = \int_{A}{\sigma_{zz}dA} = \bar{N}_{z}
\end{equation}
The stress resultant $N_{z}$ conjugates with constant strain $\epsilon_{zz}$.

The formulation above for the generalized plane strain problem shares many similarities with that of the conventional plane stress or plane strain problem. +All the differences are associated with the introduction of an additional degree of freedom for the out-of-plane direction+.

### Implementation

The out-of-plane strain is a scalar variable, and it can be added to the standard system of equations for a mechanics problem, where $\boldsymbol{u}_x$ and $\boldsymbol{u}_y$ represent
the displacement vectors in the $x$ and $y$ directions, $\boldsymbol{f}_x$ and $\boldsymbol{f}_y$ represent the corresponding reaction forces. The discussion here is for the case where the two-dimensional model lies in the $x$-$y$ plane,  The partitioned linearized system of equations, in which the block entries in the stiffness matrix are represented by subscripted $\boldsymbol{K}$ terms, can be written including the scalar strain variable as follows:

<!--This is the intended equation, but \hline was not working when the equation was created-->
<!-- \begin{equation} -->
<!-- \left[ -->
<!-- \begin{array}{cc|c} -->
<!-- \boldsymbol{K}_{xx} & \boldsymbol{K}_{xy} & \boldsymbol{K}_{xz} \\ -->
<!-- \boldsymbol{K}_{yx} & \boldsymbol{K}_{yy} & \boldsymbol{K}_{yz} \\ \hline -->
<!-- \boldsymbol{K}_{zx} & \boldsymbol{K}_{zy} & K_{zz} -->
<!-- \end{array} -->
<!-- \right] -->
<!-- \left\{ -->
<!-- \begin{array}{c} -->
<!-- \boldsymbol{u}_x \\ -->
<!-- \boldsymbol{u}_y \\ \hline -->
<!-- \epsilon_{zz} -->
<!-- \end{array} -->
<!-- \right\} -->
<!-- = -->
<!-- \left\{ -->
<!-- \begin{array}{c} -->
<!-- \boldsymbol{f}_x \\ -->
<!-- \boldsymbol{f}_y \\ \hline -->
<!-- N_{z} -->
<!-- \end{array} -->
<!-- \right\} -->
<!-- \end{equation} -->

\begin{equation}
\left[
\begin{array}{ccc}
\boldsymbol{K}_{xx} & \boldsymbol{K}_{xy} & \boldsymbol{K}_{xz} \\
\boldsymbol{K}_{yx} & \boldsymbol{K}_{yy} & \boldsymbol{K}_{yz} \\
\boldsymbol{K}_{zx} & \boldsymbol{K}_{zy} & K_{zz}
\end{array}
\right]
\left\{
\begin{array}{c}
\boldsymbol{u}_x \\
\boldsymbol{u}_y \\
\epsilon_{zz}
\end{array}
\right\}
=
\left\{
\begin{array}{c}
\boldsymbol{f}_x \\
\boldsymbol{f}_y \\
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

## List of MOOSE objects used in the implementation

Objects available for generalized plane strain:

- [Stress Divergence Kernel](/StressDivergence.md): in-plane equilibrium equation

- [Stress Models](tensor_mechanics/Stresses.md): full stress tensor calculation

Objects specific for generalized plane strain:

- [Generalized Plane Strain ScalarKernel](/GeneralizedPlaneStrain.md): out-of-plane equilibrium condition

- [Generalized Plane Strain UserObject](/GeneralizedPlaneStrainUserObject.md): residual and diagonal Jacobian calculation for scalar out-of-plane strain variable

- [Generalized Plane Strain Off-diagonal Kernel](/GeneralizedPlaneStrainOffDiag.md): in-plane displacement variables and scalar out-of-plane strain coupling

- [Strain Calculations](tensor_mechanics/Strains.md): in-plane strain calculation and formation of full strain tensor considering the scalar out-of-plane strain

## How to use generalized plane strain model

The [GeneralizedPlaneStrainAction](/GeneralizedPlaneStrainAction.md) can be used to set up a generalized plane strain model. The [TensorMechanicsAction](/TensorMechanicsAction.md) which considers the [GeneralizedPlaneStrainAction](/GeneralizedPlaneStrainAction.md) as Meta-Action can also be used.

!bibtex bibliography
