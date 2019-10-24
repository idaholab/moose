# Cohesive Zone Model

!syntax description /CohesiveZoneModel/CohesiveZoneModelAction

## Description

The `CohesiveZoneModel`action simply adds the required `CZMInterfaceKernels`.
It is till responsibility of the user to add the appropriate `CZMmaterial` block and and to split the mesh using `BreakMeshByBlockGenrator`.

The `CohesiveZoneModel` can be used for both 2D and 3D cases.


!syntax description /CohesiveZoneModel/CohesiveZoneModelAction

!syntax parameters /CohesiveZoneModel/CohesiveZoneModelAction

!syntax inputs /CohesiveZoneModel/CohesiveZoneModelAction

## Associated Actions

!syntax list /CohesiveZoneModel objects=True actions=False subsystems=False

!syntax list /CohesiveZoneModel objects=False actions=False subsystems=True

!syntax list /CohesiveZoneModel objects=False actions=True subsystems=False
