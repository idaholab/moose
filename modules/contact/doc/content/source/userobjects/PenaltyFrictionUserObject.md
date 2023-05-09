# PenaltyFrictionUserObject

## Description

The `PenaltyFrictionUserObject` object computes tangential pressures due
to Coulomb friction by looping over mortar segments. The accumulated tangential
slip is penalized by a `penalty_friction` factor. As the slip accumulates, the
frictional pressure(s) saturates at the Coulomb limit. Note that a high enough
value of the `penalty_friction` value is required to obtain realistic frictional
pressure; otherwise, the tangential model will underpredict frictional forces. That is,
higher values of the `penalty_friction` parameter will more accurately predict the stick
and slip Coulomb behavior at the expense of worsening the convergence behavior.

The tangential pressure (or pressures for a three-dimensional problem) generated
by this object contains Jacobian information as a consequence of using automatic
differentiation objects. These pressures are directly applied on the contacting
surfaces by [TangentialMortarMechanicalContact](/TangentialMortarMechanicalContact.md).
That is, the tangential contact pressure values computed by this object serve a purpose
analogous to the Lagrange multipliers when exact enforcement is used
(see [ComputeFrictionalForceLMMechanicalContact](/ComputeFrictionalForceLMMechanicalContact.md)).
This object is set up automatically when using the contact action [ContactAction](/ContactAction.md).

!syntax parameters /UserObjects/PenaltyFrictionUserObject

!syntax inputs /UserObjects/PenaltyFrictionUserObject

!syntax children /UserObjects/PenaltyFrictionUserObject
