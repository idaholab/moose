# Cohesive Zone Master Action System

!syntax description /Modules/TensorMechanics/CohesiveZoneMaster/CohesiveZoneMasterAction

## Description

The `TensorMechanics` system provides a cohesive zone modeling capability that can be used to introduce traction-separation models running at the interfaces between regions modeled with continuum finite elements. The implemented cohesive zone model (CZM) is based on a Discrete Galerkin approach and therefore does not require cohesive elements. The CZM is formulated in terms of traction separation laws and requires five ingredients:

1. `BreakMeshByBlockGenerator`
2. `DisplacementJumpProvider`
3. `CohesiveZoneConstituiveModel`
4. `EquilibriumTractionCalcualtor`
5. `CZMInterfaceKernelSmallStrain`


The [BreakMeshByBlockGenerator](BreakMeshByBlockGenerator.md) is utilized to create the cohesive zone interface  by splitting a monolithic mesh into blocks by adding the required nodes and boundaries between each block pair.
The split mesh allows to compute a displacement jump $\llbracket u \rrbracket$ at each quadrature point on the interface.

The `DisplacementJumpProvider` computes the displacement jump across the cohesive zone according to the formulatio. The `CohesiveZoneConstituiveModel` provides the cohesive zone response.
The `EquilibriumTractionCalcualtor` computes the equilibrium traction and its derivative w.r.t. the displacement jump in global coordinates, $\llbracket u \rrbracket$.
And finally, the `CZMInterfaceKernelSmallStrain` imposes equilibrium by adding the proper residual to the system, and provides the analytic Jacobian. Note, the provided `CZMInterfaceKernelSmallStrain` assume the `CohesiveZoneConstituiveModel` is only function of the displacement $\llbracket u \rrbracket$.

The system supports two different kinematic formulations:

1. `Small Strain`
2. `Total Lagrangian`

Each type of formulations requires adding a different  `DisplacementJumpProviders`, `EquilibriumTractionCalcualtor`, `CZMInterfaceKernelSmallStrain`.

The `Small Strain` formulations requires adding:

1. [CZMDisplacementJumpProviderSmallStrain](CZMDisplacementJumpProviderSmallStrain.md)
2. [EquilibriumTractionCalcualtorSmallStrain](CZMEquilibriumTractionCalculatorSmallStrain.md)
3. [CZMInterfaceKernelCalcualtorSmallStrain](CZMInterfaceKernelSmallStrain.md)

The `Total Lagrangian` formulations requires adding:

1. [DisplacementJumpProviderIncrementalTotalLagrangian](CZMDisplacementJumpProviderIncrementalTotalLagrangian.md)
2. [EquilibriumTractionCalcualtorTotalLagrangian](CZMEquilibriumTractionCalculatorTotalLagrangian.md)
3. [CZMInterfaceKernelCalcualtorTotalLagrangian](CZMInterfaceKernelTotalLagrangian.md)

The `CohesiveZoneMaster` automatically adds the proper `DisplacementJumpProvider`, `EquilibriumTractionCalcualtor`, `CZMInterfaceKernelSmallStrain` based on the `kinematic_type` parameter value (see inputs).
Note: even when using the `CohesiveZoneMaster` action it is the responsibility of the user to add the appropriate `CohesiveZoneConstituiveModel` and `BreakMeshByBlockGenrator` blocks in the input file.

Warning: to obtain a traction consistent with the bulk stress, the `Small Strain` kinematic should be used together with a bulk `Small Strain` formulation, and `Total Lagrangian` kinematics should be used together with a `Finite Strain` formulation (see )

### Cohesive zone Constitutive Models

The implemented framework allows to implement two different types of `CohesiveZoneConstituiveModel` (e.g. traction separation laws): i) path independent , and ii) path dependent models.
The first type of models only assumes the traction is only function of the interface total displacement jump, i.e. $\hat{t} =f\left(\llbracket \hat{u} \rrbracket\right)$. This kind of `CohesiveZoneConstituiveModels` models can be implemented overriding `CZMConstituiveModelTotalBase` class.
Path dependent models assumes the traction increment is a function of the interface displacement jump increment,  $\Delta\hat{t} =f\left(\Delta \llbracket \hat{u} \rrbracket\right)$. Path dependent models can be implemented overriding `CZMConstituiveModelIncrementalBase` class.
In both cases the traction separation law should always be written in terms of the local interface response assuming small strains. All the kinematics is already embedded in the `DisplacementJumpProvider` and `EquilibriumTractionCalcualtor`.
Both types of `CohesiveZoneConstituiveModel` allow using  the `Small Strain` and the `Total Lagrangian` kinematics.

!listing modules/tensor_mechanics/test/tests/czm/czm_large_deformation.i block=Modules/TensorMechanics/CohesiveZoneMaster/czm_ik

!syntax description /Modules/TensorMechanics/CohesiveZoneMaster/CohesiveZoneMasterAction
!syntax parameters /Modules/TensorMechanics/CohesiveZoneMaster/CohesiveZoneMasterAction
!syntax inputs /Modules/TensorMechanics/CohesiveZoneMaster/CohesiveZoneMasterAction

!syntax parameters /Modules/TensorMechanics/CohesiveZoneMaster/CohesiveZoneMasterAction

## Associated Actions

!syntax list /Modules/TensorMechanics/CohesiveZoneMaster objects=True actions=False subsystems=False

!syntax list /Modules/TensorMechanics/CohesiveZoneMaster objects=False actions=False subsystems=True

!syntax list /Modules/TensorMechanics/CohesiveZoneMaster objects=False actions=True subsystems=False
