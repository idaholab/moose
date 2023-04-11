# PenaltyFrictionUserObject

## Description

The `PenaltyFrictionUserObject` object computes tangential pressures due
to Coulomb friction by looping over mortar segments. The accumulated tangential
slip is penalized by a `penalty_friction` factor. As the slip accumulates, the
frictional pressure(s) saturates at the Coulomb limit. Note that a reasonable
value of the `penalty_friction` value is required to obtain realistic frictional
pressure; otherwise, the tangential model will underpredict frictional forces.

The tangential pressure (or pressures for a three-dimensional problem) generated
by this object contains Jacobian information as a consequence of using automatic
differentiation objects. These pressures are directly applied on the contacting
surfaces by [TangentialMortarMechanicalContact](/TangentialMortarMechanicalContact.md).
That is, the tangential contact pressures computed by this object serves a purpose
analogous to the Lagrange multipliers when exact enforcement is used
(see [ComputeFrictionalForceLMMechanicalContact](/ComputeFrictionalForceLMMechanicalContact.md)).
This object is set up automatically when using the contact action [ContactAction](/ContactAction.md).

!syntax description /UserObjects/PenaltyFrictionUserObject

!syntax parameters /UserObjects/PenaltyFrictionUserObject

!syntax inputs /UserObjects/PenaltyFrictionUserObject

!syntax children /UserObjects/PenaltyFrictionUserObject
