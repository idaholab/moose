# Cohesive Zone Master Action System

!syntax description /Modules/TensorMechanics/CohesiveZoneMaster/CohesiveZoneAction

## Description

The `TensorMechanics` system provides a cohesive zone modeling capability that can be used to introduce traction-separation models running at the interfaces between regions modeled with continuum finite elements. The implemented cohesive zone model (CZM) is based on a Discrete Galerkin approach and therefore does not require cohesive elements. The CZM is formulated in terms of traction separation laws and requires five ingredients:

1. `BreakMeshByBlockGenerator`
2. `ComputeDisplacementJump`
3. `ComputeLocalTraction`
4. `ComputeGlobalTraction`
5. `CZMInterfaceKernel`


The [BreakMeshByBlockGenerator](BreakMeshByBlockGenerator.md) is utilized to create the cohesive zone interface  by splitting a monolithic mesh into blocks by adding the required nodes and boundaries between each block pair. The split mesh allows to compute a displacement jump $\llbracket u \rrbracket$ at each quadrature point on the interface. The schematic below is an example of using `BreakMeshByBlockGenerator` on a 3-blocks, 2-dimensional mesh. The generated interfaces are highlighted in yellow.

!media media/tensor_mechanics/BreakMeshByBlock.png style=width:100%;

The `ComputeDisplacementJump` object computes the displacement jump across the cohesive zone according to the selected formulation. The `ComputeLocalTraction` provides the cohesive zone response in the natural interface coordinate system.
The `ComputeGlobalTraction` object computes the traction in global coordinates and its derivative w.r.t. the displacement jump in global coordinates, $\llbracket u \rrbracket$.
The `CZMInterfaceKernel` imposes equilibrium by adding the proper residual to the system, and provides the analytic Jacobian.

!alert note
The provided `CZMInterfaceKernel` assume the `ComputeLocalTraction` is only function of the displacement $\llbracket u \rrbracket$. If one wants to implement a traction separation law depending upon other variables, such as bulk stress or temperature, it is responsibility of the user to implement the proper Jacobian terms.

The `CohesiveZoneMaster` action automatically adds the proper `ComputeDisplacementJump`, `ComputeGlobalTraction`, `CZMInterfaceKernel` based on the `kinematic_type` parameter value (see inputs).
The flowchart below summarizes the flow of information of the cohesive zone modeling frameworks, and highlights the  objects automatically added by the `CohesizeZoneMaster` action.

!media media/tensor_mechanics/CZMMasterAction.png style=width:100%;

!alert note
Even when using the `CohesiveZoneMaster` action it is the responsibility of the user to add the appropriate `ComputeLocalTraction` constitutive model and `BreakMeshByBlockGenrator` in the input file.

### Supported Kinematic Formulations

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

As mentioned in previous section, the `CohesiveZoneMaster` action adds the appropriate objects depending on the selected kinematic formulation.  

!alert warning
To obtain a traction consistent with the bulk stress, the `Small Strain` kinematic should be used together with a bulk `Small Strain` formulation, and the `Total Lagrangian` kinematics should be used together with a `Finite Strain` formulation.

### Cohesive Zone Constitutive Models

The implemented framework allows to implement two different types of cohesive zone constitutive models (e.g. traction separation laws): i) path independent, and ii) path dependent.
The first type of models assumes the traction is only function of the total interface  displacement jump, i.e. $\hat{t} =f\left(\llbracket \hat{u} \rrbracket\right)$. This kind of models do not have history variables and can be implemented overriding the `ComputeLocalTractionTotalBase` class.
Instead, path dependent models have history variables and assume the traction increment is a function of the interface displacement jump increment, and its old value, $\Delta\hat{t} =f\left(\Delta \llbracket \hat{u} \rrbracket, \llbracket \hat{u} \rrbracket_{old} \right)$. Path dependent models can be implemented overriding the `ComputeLocalTractionIncrementalBase` class.
In both cases, the traction separation law should always be formulated in terms of the local interface response and assuming `Small strain`. Furthermore, all constitutive laws should be implemented for the 3D general case, even if used for 1D or 2D simulations.
The kinematic is already embedded in the `ComputeDisplacementJump` and `ComputeGlobalTraction` objects.
Both types of `ComputeLocalTraction` objects allow using either the `Small Strain` and the `Total Lagrangian` kinematics.

## Input Examples

The following example show how to use the `CohesiveZoneMaster` action.

!listing modules/tensor_mechanics/test/tests/cohesive_zone_model/stretch_rotate_large_deformation.i block=Modules/TensorMechanics/CohesiveZoneMaster

If necessary, multiple instances of the `CohesiveZoneMaster` action can be added, for instance when different material properties `base_name` are needed for different boundaries. The `base_name` parameter used in the action should also be provided to the associated materials.
The `generate_output` parameter adds scalar quantities of the traction and displacement jump to the outputs. Available options are: `traction_x traction_y traction_z normal_traction tangent_traction jump_x jump_y jump_z normal_jump tangent_jump pk1_traction_x pk1_traction_y pk1_traction_z `.
The name `traction` refers to the Cauchy traction, `pk1_traction` refers to the the first Piola-Kirchoff traction, and `jump` refers to the displacement jump across the cohesive interface. All the above vectors are defined in the global coordinate system.
The `normal_traction` and `tangent_traction` are scalar values compute using [CZMRealVectorScalar](CZMRealVectorScalar.md) (the same is true for the `normal_jump` and `tangent_jump`).

!listing modules/tensor_mechanics/test/tests/cohesive_zone_model/czm_multiple_action_and_materials.i start=[Modules/TensorMechanics/CohesiveZoneMaster] end=[Modules] include-end=false

!syntax parameters /Modules/TensorMechanics/CohesiveZoneMaster/CohesiveZoneAction
!syntax inputs /Modules/TensorMechanics/CohesiveZoneMaster/CohesiveZoneAction

## Associated Actions

!syntax list /Modules/TensorMechanics/CohesiveZoneMaster objects=True actions=False subsystems=False

!syntax list /Modules/TensorMechanics/CohesiveZoneMaster objects=False actions=False subsystems=True

!syntax list /Modules/TensorMechanics/CohesiveZoneMaster objects=False actions=True subsystems=False
