# Shifted Cohesive Zone Physics System

!syntax description /Physics/SolidMechanics/ShiftedCohesiveZone/ShiftedCohesiveZoneAction

## Description

The `ShiftedCohesiveZone` action sets up a shifted cohesive zone method (SCZM) model on a non-interface-fitted mesh. It extends the [CohesiveZone physics system](syntax/Physics/SolidMechanics/CohesiveZone/index.md) by adding the shifted interface kernels, displacement-jump material, and signed-distance objects needed to shift cohesive interface interactions onto surrogate interfaces composed of element edges or faces.

## Input Example

!listing modules/shifted_boundary_method/test/tests/shifted_cohesive_zone/rectangle_sczm_tri_material.i block=Physics/SolidMechanics/ShiftedCohesiveZone

!syntax parameters /Physics/SolidMechanics/ShiftedCohesiveZone/ShiftedCohesiveZoneAction
!syntax inputs /Physics/SolidMechanics/ShiftedCohesiveZone/ShiftedCohesiveZoneAction
