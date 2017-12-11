#Stress Divergence Kernel Settings

Within the tensor mechanics module, we have three separate ways to calculate the strain and stress:

!table id=strain_formulations caption=Consistent Strain and Stress Formulations
| Theoretical Formulation                           | Tensor Mechanics Classes    |
|---------------------------------------------------|-----------------------------|
| Linearized elasticity total small strain problems | [ComputeLinearElasticStress](/ComputeLinearElasticStress.md) and [ComputeSmallStrain](/ComputeSmallStrain.md) (in the Tensor Mechanics master action use the argument `strain = SMALL`) |
| Linearized elasticity incremental small strain    | [ComputeFiniteStrainElasticStress](/ComputeFiniteStrainElasticStress.md) and [ComputeIncrementalSmallStrain](/ComputeIncrementalSmallStrain.md) (in the Tensor Mechanics master action `strain = SMALL` and `incremental = true` )|
| Large deformation problems, including elasticity and/or plasticity | [ComputeFiniteStrainElasticStress](/ComputeFiniteStrainElasticStress.md), or other inelastic stress material class, and [ComputeFiniteStrain](/ComputeFiniteStrain.md) (in the Tensor Mechanics master action use `strain = FINITE`) |

The linearized elasticity problems are calculated on the reference mesh.  In the linearized elasticity total small strain formulation, [ComputeSmallStrain](/ComputeSmallStrain.md) a rotation increment is not used; in [ComputeIncrementalSmallStrain](/ComputeIncrementalSmallStrain.md) the rotation increment is defined as the identity tensor.  Both the total small strain and the incremental small strain classes pass to the stress divergence kernel a stress calculated on the reference mesh ( $ \sigma(X) $ ).

In the third set of the plug-and-play tensor mechanics classes, the large deformation formulation calculates the strain and stress on the deformed (current) mesh.  As an example, at the end of the [ComputeFiniteStrain](/ComputeFiniteStrain.md) class the strains are rotated to the deformed mesh, and in the [ComputeFiniteStrainElasticStress](/ComputeFiniteStrainElasticStress.md) class the stress is rotated to the deformed mesh.  Newer material models, such as crystal plasticity models and creep models, also rotate the strain and stress to the deformed mesh.  In these large deformation classes, the stress passed to the stress divergence kernel is calculated with respect to the deformed mesh ( $ \sigma(x) $ ).

As users and developers, we must take care to ensure consistency in the mesh used to calculate the strain and stress and the mesh used to calculate the residual from the stress divergence equation.  In the [StressDivergenceTensors](/StressDivergenceTensors.md) kernel, or the various stress divergence actions, the parameter `use_displaced_mesh` is used to determine if the deformed or the reference mesh should be used:

!table id=governing_equation_mesh_formulation caption=Stress Divergence Governing Equation and Mesh Configuration Correspondence
| Simulation Formulation | Governing Equation  | Correct Kernel Parameter | Mesh Configuration |
| - | - | - | - |
| Linearized Elasticity | $ \nabla_X \cdot \sigma (X) = 0 $ | `use_displaced_mesh = false` | Reference mesh (undeformed) |
| Large Deformation (both Elasticity and Inelasticity) | $ \nabla_x \cdot \sigma (x) = 0 $ | `use_displaced_mesh = true ` | Deformed mesh (current)  |

In the stress divergence kernel, nabla is given by the gradients of the test functions, and the mesh, to which the gradients are taken with respect to, is determined by the `use_displaced_mesh` parameter setting.  A source of confusion can be that the `use_displaced_mesh` parameter is not used in the materials, which compute strain and stress, but this parameter does play a large role in the calculations of the stress divergence kernel.

!!! info "Use of the Tensor Mechanics Master Action Recommended"
    The `use_displaced_mesh` parameter must be set correcting to ensure consistency in the equilibrium equation:  if the stress is calculated with respect to the deformed mesh, the test function gradients must also be calculated with respect to the deformed mesh. The Tensor Mechanics master action is designed to automatically determine and set the flag for the `use_displaced_mesh` parameter correctly for the selected strain formulation.
    We recommend that users employ the Tensor Mechanics master action whenever possible to ensure consistency between the test function gradients and the strain formulation selected.

Small strain linearized elasticity problems should be run with the parameter `use_displaced_mesh = false` in the kernel to ensure all calculations across all three classes (strain, stress, and kernel) are computed with respect to the reference mesh.

!listing modules/tensor_mechanics/tutorials/basics/part_1.1.i block=Modules/TensorMechanics/Master

Large deformation problems should be run with the parameter setting `use_displaced_mesh = true` in the kernel so that the kernel and the materials all compute variables with respect to the deformed mesh.

The input file syntax to set the Stress Divergence kernel for large deformation (finite strain) problems is

!listing modules/tensor_mechanics/test/tests/finite_strain_elastic/finite_strain_elastic_new_test.i block=Modules
