# Stress Divergence

A material varies from its rest shape due to stress. This departure from the rest shape is called
deformation or displacement, and the proportion of deformation to original size is called strain. To
determine the deformed shape and the stress, a governing equation is solved to determine the
displacement vector $\boldsymbol{u}$.

## Mathematical Introduction

The strong form of the governing equation on the domain $\Omega$ and boundary
$\Gamma=\Gamma_{\mathit{t_i}}\cup\Gamma_{\mathit{g_i}}$ can be stated as follows:
\begin{equation}
\begin{aligned}
\nabla \cdot (\boldsymbol{\sigma} + \boldsymbol{\sigma}_0) + \boldsymbol{b} =& \boldsymbol{0} \;\mathrm{in}\;\Omega \\
\boldsymbol{u} =& \boldsymbol{g}\;\mathrm{in}\;\Gamma_{ \boldsymbol{g}} \\
\boldsymbol{\sigma} \cdot \boldsymbol{n}=&\boldsymbol{t}\;\mathrm{in}\;\Gamma_{ \boldsymbol{t}}
\end{aligned}
\end{equation}
where $\boldsymbol{\sigma}$  is the Cauchy stress tensor, $\boldsymbol{\sigma}_0$
is an additional source of stress (such as pore pressure), $\boldsymbol{u}$ is
the displacement vector, $\boldsymbol{b}$ is the body force, $\boldsymbol{n}$ is
the unit normal to the boundary, $\boldsymbol{g}$ is the prescribed displacement
on the boundary and $\boldsymbol{t}$ is the prescribed traction on the boundary.
The weak form of the residual equation is expressed as:
\begin{equation}
  \mathbb{R} = \left( \boldsymbol{\sigma} + \boldsymbol{\sigma}_0, \nabla \phi_m \right) - \left< \boldsymbol{t}, \phi_m \right> - \left( \boldsymbol{b}, \phi_m \right)  = \boldsymbol{0}
\end{equation}
where $(\cdot)$ and $\left< \cdot \right>$ represent volume and boundary integrals,
respectively. The solution of the residual equation with Newton's method requires
the Jacobian of the residual equation, which can be expressed as (ignoring boundary
terms)
\begin{equation}
  \mathbb{J} = \left( \frac{\partial \boldsymbol{\sigma}}{\partial \nabla \boldsymbol{u}} , \nabla \phi_m \right),
\end{equation}
assuming $\boldsymbol{\sigma}_0$ is independent of the strain.

The material stress response is described by the constitutive model, where the stress is determined
as a function of the strain, i.e. $\tilde{\boldsymbol{\sigma}}( \boldsymbol{\epsilon} -
\boldsymbol{\epsilon}_0)$, where $\boldsymbol{\epsilon}$ is the strain and $\boldsymbol{\epsilon}_0$ is a stress
free strain. For example, in linear elasticity (only valid for small strains), the material response
is linear, i.e.  $\boldsymbol{\sigma} = \boldsymbol{\mathcal{C}}(\boldsymbol{\epsilon} - \boldsymbol{\epsilon}_0)$.

## Consistency Between Stress and Strain id=consistency_stress_strain_use_displaced_mesh

Within the solid mechanics module, we have three separate ways to consistently
calculate the strain and stress as shown in [strain_formulations]. Attention to
the correspondence among the stress and strain formulations is necessary to ensure
the stress and strain calculations are performed on the correct material configuration.

!table id=strain_formulations caption=Consistent Strain and Stress Formulations
| Theoretical Formulation                           | Solid Mechanics Classes    |
|---------------------------------------------------|-----------------------------|
| Linearized elasticity total small strain problems | [ComputeLinearElasticStress](/ComputeLinearElasticStress.md) and [ComputeSmallStrain](/ComputeSmallStrain.md) (in the [SolidMechanics/QuasiStatic](/Physics/SolidMechanics/QuasiStatic/index.md) use the argument `strain = SMALL`) |
| Linearized elasticity incremental small strain    | [ComputeFiniteStrainElasticStress](/ComputeFiniteStrainElasticStress.md) and [ComputeIncrementalStrain](/ComputeIncrementalStrain.md) (in the [SolidMechanics/QuasiStatic](/Physics/SolidMechanics/QuasiStatic/index.md) `strain = SMALL` and `incremental = true` ) |
| Large deformation problems, including elasticity and/or plasticity | [ComputeFiniteStrainElasticStress](/ComputeFiniteStrainElasticStress.md), or other inelastic stress material class, and [ComputeFiniteStrain](/ComputeFiniteStrain.md) (in the [SolidMechanics/QuasiStatic](/Physics/SolidMechanics/QuasiStatic/index.md) use `strain = FINITE`) |

### Linearized Elasticity Problems

The linearized elasticity problems are calculated on the reference mesh.  In the linearized
elasticity total small strain formulation, [ComputeSmallStrain](/ComputeSmallStrain.md) a rotation
increment is not used; in [ComputeIncrementalStrain](/ComputeIncrementalStrain.md) the
rotation increment is defined as the identity tensor.  Both the total small strain and the
incremental small strain classes pass to the stress divergence kernel a stress calculated on the
reference mesh, $\sigma(X)$.

### Large Deformation Problems

In the third set of the plug-and-play solid mechanics classes, the large deformation formulation
calculates the strain and stress on the deformed (current) mesh.  As an example, at the end of the
[ComputeFiniteStrain](/ComputeFiniteStrain.md) class the strains are rotated to the deformed mesh,
and in the [ComputeFiniteStrainElasticStress](/ComputeFiniteStrainElasticStress.md) class the stress
is rotated to the deformed mesh.  Newer material models, such as crystal plasticity models and creep
models, also rotate the strain and stress to the deformed mesh.  In these large deformation classes,
the stress passed to the stress divergence kernel is calculated with respect to the deformed mesh, $\sigma(x)$.

!alert warning title=Ensure Consistency in Stress and Strain Formulations
As users and developers, we must take care to ensure consistency in the mesh used to calculate the
strain and the mesh used to calculate the residual from the stress divergence equation.


In the [StressDivergenceTensors](/StressDivergenceTensors.md) kernel, or the various stress
divergence actions, the parameter `use_displaced_mesh` is used to determine if the deformed or the
reference mesh should be used as detailed in [governing_equation_mesh_formulation].

!table id=governing_equation_mesh_formulation caption=Stress Divergence Governing Equation and Mesh Configuration Correspondence
| Simulation Formulation | Governing Equation  | Correct Kernel Parameter | Mesh Configuration |
| - | - | - | - |
| Linearized Elasticity | $\nabla_X \cdot \sigma (X) = 0$ | `use_displaced_mesh = false` | Reference mesh (undeformed) |
| Large Deformation (both Elasticity and Inelasticity) | $\nabla_x \cdot \sigma (x) = 0$ | `use_displaced_mesh = true` | Deformed mesh (current)  |

In the stress divergence kernel, $\nabla$ is given by the gradients of the test functions, and the mesh,
which the gradients are taken with respect to, is determined by the `use_displaced_mesh` parameter
setting.  A source of confusion can be that the `use_displaced_mesh` parameter is not used in the
materials, which compute strain and stress, but this parameter does play a large role in the
calculations of the stress divergence kernel.

## QuasiStatic Physics in Solid Mechanics

The `use_displaced_mesh` parameter must be set correcting to ensure consistency in the equilibrium
equation: if the stress is calculated with respect to the deformed mesh, the test function gradients
must also be calculated with respect to the deformed mesh. The [SolidMechanics/QuasiStatic](/Physics/SolidMechanics/QuasiStatic/index.md) is
designed to automatically determine and set the flag for the `use_displaced_mesh` parameter correctly
for the selected strain formulation.

!alert note title=Use of the Solid Mechanics QuasiStatic Physics Recommended
We recommend that users employ the +[SolidMechanics/QuasiStatic](/Physics/SolidMechanics/QuasiStatic/index.md)+
whenever possible to ensure consistency between the test function gradients and
the strain formulation selected.

### Linearized Elasticity Problems

Small strain linearized elasticity problems should be run with the parameter `use_displaced_mesh =
false` in the kernel to ensure all calculations across all three classes (strain, stress, and kernel)
are computed with respect to the reference mesh. These settings are automatically
handled with the [SolidMechanics/QuasiStatic](/Physics/SolidMechanics/QuasiStatic/index.md) use.

!listing modules/solid_mechanics/tutorials/basics/part_1.1.i block=Physics/SolidMechanics/QuasiStatic

### Large Deformation Problems

Large deformation problems should be run with the parameter setting `use_displaced_mesh = true` in
the kernel so that the kernel and the materials all compute variables with respect to the deformed
mesh; however, the setting of `use_displaced_mesh` should not be changed from the default
in the materials.
The [SolidMechanics/QuasiStatic](/Physics/SolidMechanics/QuasiStatic/index.md) automatically creates the appropriate settings for all classes.
The input file syntax to set the Stress Divergence kernel for finite strain problems is:

!listing modules/solid_mechanics/test/tests/finite_strain_elastic/finite_strain_elastic_new_test.i
         block=Physics
