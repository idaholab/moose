# CohesiveZoneMortarUserObjectAux

## Description

The `CohesiveZoneMortarUserObjectAux` outputs various cohesive zone quantities that
are helpful for analyzing or debugging cohesive zone or debonding simulation results.
This auxiliary kernel is to be used in conjunction with mortar cohesive zone user objects, namely
[BilinearMixedModeCohesiveZoneModel](/BilinearMixedModeCohesiveZoneModel.md).

Quantities that this object can output include:

- `mode_mixity_ratio`: shear-to-opening mode mixity ratio.
- `cohesive_damage`: scalar damage variable.
- `local_normal_jump`: local normal displacement jump.
- `local_tangential_jump`: legacy name for the first local tangential displacement jump component.
- `local_tangential_jump_1`: first local tangential displacement jump component.
- `local_tangential_jump_2`: second local tangential displacement jump component.
- `local_tangential_jump_effective`: magnitude of the local tangential displacement jump,
  `sqrt(t1^2 + t2^2)`.
- `cohesive_traction_normal`: local normal cohesive traction component.
- `cohesive_traction_tangential_1`: first local tangential cohesive traction component.
- `cohesive_traction_tangential_2`: second local tangential cohesive traction component.
- `cohesive_traction_tangential_magnitude`: magnitude of the local tangential cohesive traction,
  `sqrt(t1^2 + t2^2)`.
- `cohesive_traction_effective`: full local cohesive traction norm,
  `sqrt(tn^2 + t1^2 + t2^2)`.

The local traction outputs follow the residual sign convention used by the mortar cohesive zone
model. With the current convention, tensile opening gives a positive `local_normal_jump` and a
negative `cohesive_traction_normal`.

!syntax parameters /AuxKernels/CohesiveZoneMortarUserObjectAux

!syntax inputs /AuxKernels/CohesiveZoneMortarUserObjectAux

!syntax children /AuxKernels/CohesiveZoneMortarUserObjectAux
