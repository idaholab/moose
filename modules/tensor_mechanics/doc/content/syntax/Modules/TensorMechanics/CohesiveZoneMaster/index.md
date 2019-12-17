# Cohesive Zone Master Action System

## Description

The `TensorMechanics` system provides a cohesive zone modeling capability that can be used to introduce traction-separation models running at the interfaces between regions modeled with continuum finite elements. The implemented cohesive zone model (CZM) is based on a Discrete Galerkin approach and therefore does not require cohesive elements. The CZM is formulated in terms of traction separation laws and requires three ingredients:

1. `BreakMeshByBlockGenerator`
2. `CZMInterfaceKernels`
3. `CZMMaterials`

The [BreakMeshByBlockGenerator](BreakMeshByBlockGenerator.md) is utilized to create the cohesive zone interface  by splitting a monolithic mesh into blocks by adding the required nodes and boundaries between each block pair.
The split mesh allows to compute a displacement jump $\Delta u$ at each quadrature point on the interface.

### Small Deformations

A [CZMInterfaceKernel](CZMInterfaceKernel.md) utilizes the displacement jump $\Delta u$ to add the appropriate residual to the system and to provide the correct Jacobian. The current implementation of the `CZMInterfaceKernel` assumes the traction separation laws being only dependent from the displacement variables. Note that one `CZMInterfaceKernel` is required for each displacement component. The `CohesiveZoneMaster` block is used to simplify the process of setting up the necessary inputs required for cohesive zone modeling. It creates the appropriate number of `CZMInterfaceKernel` objects for the model dimensionality.

A `CZMMaterial` is used to compute the traction $T$ as function of the displacement jump $\Delta u$ and to provide the traction derivative $\frac{dT}{d \Delta u}. It should be noted that one can use different traction separation laws for different interface boundaries while using the same `CZMInterfaceKernels`.
Furthermore, additional traction separation laws can be implemented by deriving from the `CZMMaterialBase` class (see for instance [SalehaniIrani3DCTraction](SalehaniIrani3DCTraction.md)).

Even when using the `CohesiveZoneMaster` action it is the responsibility of the user to add the appropriate `CZMMaterial` and `BreakMeshByBlockGenrator` blocks in the input file.

!syntax description /Modules/TensorMechanics/CohesiveZoneMaster/CohesiveZoneMasterAction
!syntax parameters /Modules/TensorMechanics/CohesiveZoneMaster/CohesiveZoneMasterAction
!syntax inputs /Modules/TensorMechanics/CohesiveZoneMaster/CohesiveZoneMasterAction

### Large Deformations

Large deformations have not been implemented yet. An error occurs when the user tries to manually set `use_displaced_mesh = true` in a  `CZMMaterial` or in a `CZMInterfaceKernel`.

## Associated Actions

!syntax list /Modules/TensorMechanics/CohesiveZoneMaster objects=True actions=False subsystems=False
!syntax list /Modules/TensorMechanics/CohesiveZoneMaster objects=False actions=False subsystems=True
!syntax list /Modules/TensorMechanics/CohesiveZoneMaster objects=False actions=True subsystems=False
