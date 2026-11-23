# MortarGenericTraction

The `MortarGenericTraction` class is used to apply global traction
vectors on domain boundaries using a mortar formulation. This object
queries an interface traction vector from a mortar user object and
applies it.

When the traction comes from a cohesive zone model such as
[BilinearMixedModeCohesiveZoneModel](/BilinearMixedModeCohesiveZoneModel.md), both objects run on
the undisplaced mesh, and the traction is integrated over the undisplaced mortar segments as a
force per unit reference area. [!param](/Constraints/MortarGenericTraction/use_displaced_mesh) must
match the setting of the cohesive zone user object.

!listing modules/contact/test/tests/cohesive_zone_model/mortar_czm.i block=Constraints

!syntax description /Constraints/MortarGenericTraction

!syntax parameters /Constraints/MortarGenericTraction

!syntax inputs /Constraints/MortarGenericTraction

!syntax children /Constraints/MortarGenericTraction
