# LMWeightedVelocitiesUserObject

## Description

The `LMWeightedVelocitiesUserObject` object provides the Lagrange multiplier and
interpolation function for the enforcement of mortar mechanical constraints.
In essence, the Lagrange multiplier is provided to [TangentialMortarMechanicalContact](/TangentialMortarMechanicalContact.md) to enforce friction via a Coulomb model. This object is set up automatically
when using the contact action [ContactAction](/ContactAction.md)

For supported quasistatic local-basis Coulomb contact with the `mortar` formulation,
Jacobian-bearing evaluations include displacement derivatives of the secondary nodal normals in
the weighted gap and of their derived tangent basis in the weighted tangential velocities by
default. Stored directions and residual values remain unchanged, and residual-only mode stores no
AD derivatives. See
[ComputeWeightedGapLMMechanicalContact](/ComputeWeightedGapLMMechanicalContact.md) for the
supported contact paths, element types, one-ring sparsity and AD storage considerations, and
geometric terms whose derivatives remain frozen.

!syntax parameters /UserObjects/LMWeightedVelocitiesUserObject

!syntax inputs /UserObjects/LMWeightedVelocitiesUserObject

!syntax children /UserObjects/LMWeightedVelocitiesUserObject
