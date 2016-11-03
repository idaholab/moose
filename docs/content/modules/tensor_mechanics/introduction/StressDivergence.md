#Stress Divergence Kernel Settings

Within the tensor mechanics module, we have three separate ways to calculate the strain and stress:

1. Linearized elasticity total small strain problems: use [ComputeSmallStrain](/Materials/ComputeSmallStrain.md) and [ComputeLinearElasticStress](/Materials/ComputeLinearElasticStress.md)
2. Linearized elasticity incremental small strain: use [ComputeIncrementalSmallStrain](/Materials/ComputeIncrementalSmallStrain.md) and [ComputeFiniteStrainElasticStress](/Materials/ComputeFiniteStrainElasticStress.md)
3. Large deformation problems, including elasticity and/or plasticity: use [ComputeFiniteStrain](/Materials/ComputeFiniteStrain.md) and [ComputeFiniteStrainElasticStress](/Materials/ComputeFiniteStrainElasticStress.md)

The linearized elasticity problems are calculated on the reference mesh.  In the linearized elasticity total small strain formulation, [ComputeSmallStrain](/Materials/ComputeSmallStrain.md) a rotation increment is not used; in [ComputeIncrementalSmallStrain](/Materials/ComputeIncrementalSmallStrain.md) the rotation increment is defined as the identity tensor.  Both the total small strain and the incremental small strain classes pass to the stress divergence kernel a stress calculated on the reference mesh ( $ \sigma(X) $ ).

In the third set of the plug-and-play tensor mechanics classes, the large deformation formulation calculates the strain and stress on the deformed (current) mesh.  As an example, at the end of the [ComputeFiniteStrain](/Materials/ComputeFiniteStrain.md) class the strains are rotated to the deformed mesh, and in the [ComputeFiniteStrainElasticStress](/Materials/ComputeFiniteStrainElasticStress.md) class the stress is rotated to the deformed mesh.  Newer material models, such as crystal plasticity models and creep models, also rotate the strain and stress to the deformed mesh.  In these large deformation classes, the stress passed to the stress divergence kernel is calculated with respect to the deformed mesh ( $ \sigma(x) $ ).

As users and developers, we must take care to ensure consistency in the mesh used to calculate the strain and stress and the mesh used to calculate the residual from the stress divergence equation.  In the [StressDivergenceTensors](/Kernels/StressDivergenceTensors.md) kernel, or the various stress divergence actions, the parameter `use_displaced_mesh` is used to determine if the deformed or the reference mesh should be used:

| Simulation Formulation | Governing Equation Form  | Correct Kernel Parameter | Mesh Configuration Used |
| - | - | - | - |
| Linearized Elasticity | $ \nabla_X \cdot \sigma (X) $ | `use_displaced_mesh = false` | Reference (undeformed) mesh |
| Large Deformation (Elasticity and Plasticity) | $ \nabla_x \cdot \sigma (x) $ | `use_displaced_mesh = true ` | Deformed (current) mesh |

In the stress divergence kernel, nabla is given by the gradients of the test functions, and the mesh, to which the gradients are taken with respect to, is determined by the `use_displaced_mesh` parameter setting.  A source of confusion can be that the `use_displaced_mesh` parameter is not used in the materials, which compute strain and stress, but this parameter does play a large role in the calculations of the stress divergence kernel.

!!! info
    The `use_displaced_mesh` parameter must be set correcting to ensure consistency in the equilibrium equation:  if the stress is calculated with respect to the deformed mesh, the test function gradients must also be calculated with respect to the deformed mesh.

Small strain linearized elasticity problems should be run with the parameter `use_displaced_mesh = false` in the kernel to ensure all calculations across all three classes (strain, stress, and kernel) are computed with respect to the reference mesh.

!input modules/tensor_mechanics/tutorials/basics/part_1.1.i block=Kernels overflow-y=scroll max-height=300px

!!! important
    Large deformation problems should be run with the parameter setting `use_displaced_mesh = true` in the kernel so that the kernel and the materials all compute variables with respect to the deformed mesh.

The input file syntax to set the Stress Divergence kernel for large deformation (finite strain) problems is

!input modules/tensor_mechanics/tests/finite_strain_elastic/finite_strain_elastic_new_test.i block=Kernels overflow-y=scroll max-height=300px
