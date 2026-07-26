# Balance of Linear Momentum

## Overview id=overview

The stress-divergence kernels enforce the balance of linear momentum: they solve for the displacement
field $\boldsymbol{u}$ that puts the body in mechanical equilibrium under the applied loads.  The
module provides two equivalent formulations of this balance [!cite](10.1145/3716308):

- the [total Lagrangian](TotalLagrangianStressDivergence.md) formulation, which enforces equilibrium
  on the *reference* configuration using the first Piola-Kirchhoff stress $P_{iJ}$; and
- the [updated Lagrangian](UpdatedLagrangianStressDivergence.md) formulation, which enforces
  equilibrium on the *current* configuration using the Cauchy stress $\sigma_{ij}$.

Both consume the deformation measures produced by the [kinematics](Kinematics.md) calculator and the
stress produced by a constitutive model.  This page describes the governing equations and the
relationship between the two formulations; the detailed residual, Jacobian, and stabilization terms
for each kernel are given on the linked source pages.

## Strong and Weak Form id=strong_weak

For large deformation kinematics the total Lagrangian formulation solves
\begin{equation}
      \begin{aligned}
      P_{iJ,J}+b_{i} &= 0 & &\mathrm{on}\ \Omega_{0}\\
      P_{iJ}N_{J} &= \hat{t}_{i} & &\mathrm{on}\ \partial\Omega_{0,t}\\
      u_{i} &= \hat{u}_{i} & &\mathrm{on}\ \partial\Omega_{0,u}
      \end{aligned}
\end{equation}
where $P_{iJ}$ is the first Piola-Kirchhoff stress, $b_i$ the body force, $N_J$ the
reference-configuration normal, $\hat{t}_i$ the prescribed traction, and $\hat{u}_i$ the prescribed
displacement.  The updated Lagrangian formulation solves the equivalent problem on the current
configuration
\begin{equation}
      \begin{aligned}
      \sigma_{ij,j}+b_{i} &= 0 & &\mathrm{on}\ \Omega\\
      \sigma_{ij}n_{j} &= \hat{t}_{i} & &\mathrm{on}\ \partial\Omega_{t}\\
      u_{i} &= \hat{u}_{i} & &\mathrm{on}\ \partial\Omega_{u}
      \end{aligned}
\end{equation}
with $\sigma_{ij}$ the Cauchy stress and $n_j$ the current-configuration normal.

Multiplying the strong form by a test function $\phi^{\alpha}$ and integrating by parts gives the
weak form.  The stress-divergence kernels provide the stress term of the residual,
\begin{equation}
   R^{\alpha} = \int_{\Omega_0} P_{iJ}\, \phi^{\alpha}_{i,J} \; dV
\end{equation}
for the total Lagrangian formulation, and the analogous integral of the Cauchy stress over the
current configuration for the updated Lagrangian formulation; body forces and tractions are
contributed by separate kernels and boundary conditions.  Solving the residual with Newton's method
requires its Jacobian, the derivative of the stress term with respect to the displacement, which is
built from the constitutive tangent supplied by the material model.  The exact residual and Jacobian
for each kernel, including the $\bar{\boldsymbol{F}}$ stabilization terms, are given on the
[total Lagrangian](TotalLagrangianStressDivergence.md) and
[updated Lagrangian](UpdatedLagrangianStressDivergence.md) source pages.

## Total vs. Updated Lagrangian id=total_vs_updated

The two formulations are related through
\begin{equation}
   P_{iK} = J\, \sigma_{ij}\, F^{-1}_{Kj} ,
\end{equation}
with $J = \det F$ the volume change.  If the boundary conditions, body force, and constitutive model
are the same, the total and updated Lagrangian formulations produce *exactly the same* result; the
choice between them is one of convenience.  It is most natural to define a constitutive model that
returns the first Piola-Kirchhoff stress as a function of the deformation gradient, $P_{iJ}(F_{kL})$,
for the total Lagrangian kernel, and the Cauchy stress $\sigma_{ij}(F_{kL})$ for the updated
Lagrangian kernel.  The material system converts between the Cauchy and first Piola-Kirchhoff stress
automatically (see [`ComputeLagrangianStressCauchy`](ComputeLagrangianStressCauchy.md) and
[`ComputeLagrangianStressPK1`](ComputeLagrangianStressPK1.md)), so a model written for either measure
can be used with either kernel.

One case does favor a specific formulation: the [homogenization system](Homogenization.md) works only
with the total Lagrangian kernel, because of the difficulty of including the extra macroscopic
gradient in the current-configuration kinematics.

## Small Deformation id=small_deformation

Under small deformation kinematics the reference and current configurations coincide and both
formulations degenerate to the same problem
\begin{equation}
      \begin{aligned}
      s_{ij,j}+b_{i} &= 0 & &\mathrm{on}\ \Omega\\
      s_{ij}n_{j} &= \hat{t}_{i} & &\mathrm{on}\ \partial\Omega_{t}\\
      u_{i} &= \hat{u}_{i} & &\mathrm{on}\ \partial\Omega_{u}
      \end{aligned}
\end{equation}
where $s_{ij}$ is the small (engineering) stress and there is no distinction between the current and
reference configurations.  Both kernels support both small and large deformation theory.  The
kinematic theory is selected once, on the strain calculator, through the `large_kinematics` option
(see [The `large_kinematics` Option](Kinematics.md#large_kinematics)); the kernels derive their
kinematic theory from the strain calculator automatically.

## The `use_displaced_mesh` Flag id=use_displaced_mesh

In MOOSE the `use_displaced_mesh` flag selects whether test-function gradients and integrals are taken
with respect to the current (`true`) or reference (`false`) configuration.  In the Lagrangian system
the only object that requires `use_displaced_mesh = true` is the
[updated Lagrangian kernel](UpdatedLagrangianStressDivergence.md) when `large_kinematics = true`,
because it enforces equilibrium on the current configuration; the kernel checks this condition and
errors if it is not met.  Every other object in the system uses `use_displaced_mesh = false`.

The [SolidMechanics/QuasiStatic](/Physics/SolidMechanics/QuasiStatic/index.md) action sets this flag
correctly for the chosen formulation, and using it is the recommended way to keep the configuration on
which equilibrium is enforced consistent with the stress measure.

## Coupling to the Kernels id=coupling

Objects that need the stress computed by the stress-divergence kernels should couple to the
`cauchy_stress` or `pk1_stress` material property, as appropriate.

## Implementation id=implementation

The balance of linear momentum is enforced by the following kernels:

| Object | Use |
|--------|-----|
| [`TotalLagrangianStressDivergence`](TotalLagrangianStressDivergence.md) | reference-configuration (PK1) equilibrium, 3D and 2D Cartesian |
| [`TotalLagrangianStressDivergenceAxisymmetricCylindrical`](TotalLagrangianStressDivergenceAxisymmetricCylindrical.md) | total Lagrangian, 2D axisymmetric cylindrical |
| [`TotalLagrangianStressDivergenceCentrosymmetricSpherical`](TotalLagrangianStressDivergenceCentrosymmetricSpherical.md) | total Lagrangian, 1D centrosymmetric spherical |
| [`TotalLagrangianWeakPlaneStress`](TotalLagrangianWeakPlaneStress.md) | total Lagrangian weak plane stress |
| [`HomogenizedTotalLagrangianStressDivergence`](HomogenizedTotalLagrangianStressDivergence.md) | total Lagrangian with the [homogenization system](Homogenization.md) |
| [`UpdatedLagrangianStressDivergence`](UpdatedLagrangianStressDivergence.md) | current-configuration (Cauchy) equilibrium, Cartesian |

Each kernel provides the residual for one displacement component, so one kernel per coordinate
direction is required.  In most cases the kernels are added automatically by the
[SolidMechanics/QuasiStatic](/Physics/SolidMechanics/QuasiStatic/index.md) action.  They can also be
added directly:

!listing modules/solid_mechanics/test/tests/lagrangian/cartesian/total/patch/large_patch.i
         block=Kernels

!bibtex bibliography
