# Theory for the New Lagrangian Kernels

## Introduction

The new kernel system provides two options for enforcing stress equilibrium
over a domain described by a map between the reference configuration of the body,
defined as $\Omega_0$ with coordinates $\boldsymbol{X}$, and the current
configuration of the body defined as $\Omega$ with coordinates
$\boldsymbol{x}$.
The mathematics here uses upper case letters to define quantities
related to the reference configuration and lower case letters to define
quantities related to the current configuration.  Specifically,
\begin{equation}
      f_{i,j}=\frac{\partial f_{i}}{\partial x_{j}}
\end{equation}
denotes a spatial gradient with respect to the current coordinates
while
\begin{equation}
      f_{i,J}=\frac{\partial f_{i}}{\partial X_{j}}
\end{equation}
denotes a gradient with respect to the reference coordinates.  [potatoes]
describes these two frames of reference. The new kernel system current supports three coordinate systems: [GradientOperator.md#3D_cartesian], [GradientOperator.md#2D_axisymmetric_cylindrical], and [GradientOperator.md#1D_centrosymmetric_spherical].

!media tensor_mechanics/mechanics_potatoes.png
       id=potatoes
       style=width:30%;float:center;padding-top:1.5%;
       caption=Summary of the reference and current configurations with key quantities.

The [total Lagrangian](kernels/lagrangian/TotalLagrangianStressDivergence.md) theory
enforces the equilibrium condition mapped back to the reference
configuration, weakly solving the differential equation:
\begin{equation}
      \begin{align*}
      P_{iJ,J}+b_{i} & = & 0 & \, \mathrm{{on}\, \Omega_{0}}\\
      P_{iJ}N_{j} & = & \hat{t}_{i} & \, \mathrm{on}\, \partial\Omega_{0,n}\\
      u_{i} & = & \hat{u}_{i} & \, \mathrm{on}\, \partial\Omega_{0,e}
      \end{align*}
\end{equation}
where $P_{iJ}$ is the 1st Piola-Kirchhoff stress, $b_i$ are the body
forces in the updated configuration, $N_{j}$ are the boundary normals
in the reference configuration, $\hat{t}_{i}$ are the tractions
in the updated configuration, $u_i$ is the displacement field, and
$\hat{u}_{i}$ are imposed displacement boundary conditions.

The [updated Lagrangian](/UpdatedLagrangianStressDivergence.md) theory
enforces the equilibrium condition on the current configuration:
\begin{equation}
      \begin{align*}
      \sigma_{ij,j}+b_{i} & = & 0 & \, \mathrm{{on}\, \Omega}\\
      \sigma_{ij}n_{j} & = & \, \hat{t}_{i} & \mathrm{on}\, \partial\Omega_{n}\\
      u_{i} & = & \hat{u}_{i} & \, \mathrm{on}\, \partial\Omega_{e}
      \end{align*}
\end{equation}
where $\sigma_{ij}$ is the Cauchy stress.

It is more natural and common to define the constitutive response for the
total Lagrangian formulation as the 1st Piola Kirchhoff stress as a
function of the deformation gradient, $P_{iJ}\left(F_{kL}\right)$
updated Lagrangian formulation as the Cauchy stress as a
function of the deformation gradient, $\sigma_{ij}\left(F_{kL}\right)$, with
the deformation gradient defined as
\begin{equation}
      F_{iJ} = \delta_{i,J} + u_{i,J}
\end{equation}
with $\delta$ the Kronecker Delta.
However, the Cauchy stress and the 1st Piola Kirchhoff stress are related
\begin{equation}
     P_{iK}=J\sigma_{ij}F_{Kj}^{-1}
\end{equation}
and so it is possible to convert a "native" Cauchy stress constitutive
model to the 1st Piola-Kirchhoff stress and vice versa.

*If the boundary conditions, body force, and constitutive model are
all identical then the updated and total Lagrangian formulations will return
exactly the same results.  There is no difference in the final results
when using a [updated Lagrangian](kernels/lagrangian/UpdatedLagrangianStressDivergence.md) or
[total Lagrangian](kernels/lagrangian/TotalLagrangianStressDivergence.md) model.*

However, at times one formulation may be more convenient than another.
For example, the [homogenization system](tensor_mechanics/Homogenization.md)
system only works with the [total Lagrangian kernel](kernels/lagrangian/TotalLagrangianStressDivergence.md)
because of the difficulty in including the extra homogenization
field in the kinematic spatial derivatives.
In theory it can be more efficient to couple a "native" Cauchy stress
constitutive model to the updated Lagrangian configuration and a
"native" 1st Piola-Kirchhoff model to the total Lagrangian configuration.
However, the currently-implemented
[material system](tensor_mechanics/NewMaterialSystem.md) always coverts
Cauchy stress to 1st Piola-Kirchhoff stress and vice-versa so that models
can be used with either the updated or total Lagrangian kernels.

Both of these models degenerate to the same formulation for small kinematic
theory:
\begin{equation}
      \begin{align*}
      s_{ij,j}+b_{i} & = & 0 & \, \mathrm{{on}\, \Omega}\\
      s_{ij}n_{j} & = & \, \hat{t}_{i} & \mathrm{on}\, \partial\Omega_{n}\\
      u_{i} & = & \hat{u}_{i} & \, \mathrm{on}\, \partial\Omega_{e}
      \end{align*}
\end{equation}
where $s_{ij}$ is the small (engineering) stress and there is now
no difference between the current and reference configurations.
Both the updated and total kernels both both large and small deformation
theories.  The user can switch between them by setting the
`large_kinematics` flag to `true` for large deformation theory and
`false` for small deformation theory.  This flag applies to the kernels,
the [strain calculator](materials/lagrangian/ComputeLagrangianStrain.md), the
[homogenization system](/tensor_mechanics/Homogenization.md), and
for many models, the [constiutive](tensor_mechanics/NewMaterialSystem.md) models for calculating the stress.
 As such, it is often convenient to set it in the `GlobalParams` section
 of the input file so that you can easily switch between the two.

## The `use_displaced_mesh` Flag

In MOOSE the `use_displaced_mesh` flag indicates whether gradients
and integrals are with respect to the current (`true`) or reference (`false`)
configurations.
 With the new Lagrangian
the only object that needs to have `use_displaced_mesh`
set is the [updated Lagrangian](kernels/lagrangian/UpdatedLagrangianStressDivergence.md)
when `large_kinematics = true`.  The kernel enforces this condition with
an error.  For all other objects in the new mechanics system the
flag should be set to `false`.

The [TensorMechanics/MasterAction](/Modules/TensorMechanics/Master/index.md)
can be used to easily coordinate the
values of this flag and the `use_displaced_mesh` flag.

## Coupling to the Lagrangian kernels 

Modules expecting to couple to the `stress` material property, 
which represented the Cauchy stress in the old material system, 
should now couple to `cauchy_stress` or `pk1_stress` instead.
