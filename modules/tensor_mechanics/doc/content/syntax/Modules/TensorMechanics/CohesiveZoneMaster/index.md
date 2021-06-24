# Cohesive Zone Master Action System

!syntax description /Modules/TensorMechanics/CohesiveZoneMaster/CohesiveZoneMasterAction

## Description

The `TensorMechanics` system provides a cohesive zone modeling capability that can be used to introduce traction-separation models running at the interfaces between regions modeled with continuum finite elements. The implemented cohesive zone model (CZM) is based on a Discrete Galerkin approach and therefore does not require cohesive elements. The CZM is formulated in terms of traction separation laws and requires five ingredients:

1. `BreakMeshByBlockGenerator`
2. `ComputeDisplacementJump`
3. `ComputeLocalTraction`
4. `ComputeGlobalTraction`
5. `CZMInterfaceKernel`


The [BreakMeshByBlockGenerator](BreakMeshByBlockGenerator.md) is utilized to create the cohesive zone interface  by splitting a monolithic mesh into blocks by adding the required nodes and boundaries between each block pair.
The split mesh allows to compute a displacement jump $\llbracket u \rrbracket$ at each quadrature point on the interface.

The `ComputeDisplacementJump` object computes the displacement jump across the cohesive zone according to the selected formulation. The `ComputeLocalTraction` provides the cohesive zone response in the natural interface coordinate system.
The `ComputeGlobalTraction` object computes the traction in global coordinates and its derivative w.r.t. the displacement jump in global coordinates, $\llbracket u \rrbracket$.
And finally, the `CZMInterfaceKernel` imposes equilibrium by adding the proper residual to the system, and provides the analytic Jacobian. Note, the provided `CZMInterfaceKernel` assume the `ComputeLocalTraction` is only function of the displacement $\llbracket u \rrbracket$.

The system supports two different kinematic formulations:

1. `Small Strain`
2. `Total Lagrangian`

Each type of formulations requires adding a different  `ComputeDisplacementJump`, `ComputeGlobalTraction`, `CZMInterfaceKernel`.

The `Small Strain` formulations requires adding:

1. [CZMComputeDisplacementJumpSmallStrain](CZMComputeDisplacementJumpSmallStrain.md)
2. [CZMComputeGlobalTractionSmallStrain](CZMComputeGlobalTractionSmallStrain.md)
3. [CZMInterfaceKernelSmallStrain](CZMInterfaceKernelSmallStrain.md)

The `Total Lagrangian` formulations requires adding:

1. [CZMComputeDisplacementJumpTotalLagrangian](CZMComputeDisplacementJumpTotalLagrangian.md)
2. [CZMComputeGlobalTractionTotalLagrangian](CZMComputeGlobalTractionTotalLagrangian.md)
3. [CZMInterfaceKernelTotalLagrangian](CZMInterfaceKernelTotalLagrangian.md)

The `CohesiveZoneMaster` action automatically adds the proper `ComputeDisplacementJump`, `ComputeGlobalTraction`, `CZMInterfaceKernel` based on the `kinematic_type` parameter value (see inputs).
Note: even when using the `CohesiveZoneMaster` action it is the responsibility of the user to add the appropriate `ComputeLocalTraction` model and `BreakMeshByBlockGenrator` blocks in the input file.

Warning: to obtain a traction consistent with the bulk stress, the `Small Strain` kinematic should be used together with a bulk `Small Strain` formulation, and the `Total Lagrangian` kinematics should be used together with a `Finite Strain` formulation.

### Cohesive zone Constitutive Models

The implemented framework allows to implement two different types of cohesive zone constitutive models (e.g. traction separation laws): i) path independent , and ii) path dependent models.
The first type of models assumes the traction is only function of the interface total displacement jump, i.e. $\hat{t} =f\left(\llbracket \hat{u} \rrbracket\right)$. This kind of  models can be implemented overriding `ComputeLocalTractionTotal` class.
Path dependent models assumes the traction increment is a function of the interface displacement jump increment,  $\Delta\hat{t} =f\left(\Delta \llbracket \hat{u} \rrbracket\right)$. Path dependent models can be implemented overriding `ComputeLocalTractionIncremental` class.
In both cases the traction separation law should always be written in terms of the local interface response assuming small strains. All the kinematics is already embedded in the `ComputeDisplacementJump` and `ComputeGlobalTraction` objects.
Both types of `ComputeLocalTraction` objects allow using  the `Small Strain` and the `Total Lagrangian` kinematics.

!listing modules/tensor_mechanics/test/tests/cohesive_zone_model/czm_large_deformation.i block=Modules/TensorMechanics/CohesiveZoneMaster/czm_ik

!syntax description /Modules/TensorMechanics/CohesiveZoneMaster/CohesiveZoneMasterAction
!syntax parameters /Modules/TensorMechanics/CohesiveZoneMaster/CohesiveZoneMasterAction
!syntax inputs /Modules/TensorMechanics/CohesiveZoneMaster/CohesiveZoneMasterAction

!syntax parameters /Modules/TensorMechanics/CohesiveZoneMaster/CohesiveZoneMasterAction

## Associated Actions

!syntax list /Modules/TensorMechanics/CohesiveZoneMaster objects=True actions=False subsystems=False

!syntax list /Modules/TensorMechanics/CohesiveZoneMaster objects=False actions=False subsystems=True

!syntax list /Modules/TensorMechanics/CohesiveZoneMaster objects=False actions=True subsystems=False
