# Cohesive Zone Model

!syntax description /Modules/TensorMechanics/CohesiveZoneMaster/CohesiveZoneModelAction

## Description

The `CohesiveZoneModel`action simply adds the required `CZMInterfaceKernels`.
It is the responsibility of the user to add the appropriate `CZMmaterial` block and and to split the mesh using `BreakMeshByBlockGenrator`.

The `CohesiveZoneModel` can be used for both 2D and 3D cases.


!syntax description /Modules/TensorMechanics/CohesiveZoneMaster/CohesiveZoneModelAction
!syntax parameters /Modules/TensorMechanics/CohesiveZoneMaster/CohesiveZoneModelAction
!syntax inputs /Modules/TensorMechanics/CohesiveZoneMaster/CohesiveZoneModelAction

## Associated Actions

!syntax list /Modules/TensorMechanics/CohesiveZoneMaster objects=True actions=False subsystems=False
!syntax list /Modules/TensorMechanics/CohesiveZoneMaster objects=False actions=False subsystems=True
!syntax list /Modules/TensorMechanics/CohesiveZoneMaster objects=False actions=True subsystems=False
