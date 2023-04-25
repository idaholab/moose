# LMWeightedVelocitiesUserObject

## Description

The `LMWeightedVelocitiesUserObject` object provides the Lagrange multiplier and
interpolation function for the enforcement of mortar mechanical constraints.
In essence, the Lagrange multiplier is provided to [TangentialMortarMechanicalContact](/TangentialMortarMechanicalContact.md) to enforce friction via a Coulomb model. This object is set up automatically
when using the contact action [ContactAction](/ContactAction.md)

!syntax parameters /UserObjects/LMWeightedVelocitiesUserObject

!syntax inputs /UserObjects/LMWeightedVelocitiesUserObject

!syntax children /UserObjects/LMWeightedVelocitiesUserObject
