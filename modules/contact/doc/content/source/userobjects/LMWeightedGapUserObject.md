# LMWeightedGapUserObject

## Description

The `LMWeightedGapUserObject` object provides the Lagrange multiplier and
interpolation function for the enforcement of mortar mechanical constraints.
In essence, the Lagrange multiplier is provided to [NormalMortarMechanicalContact](/NormalMortarMechanicalContact.md) to enforce the non-penetration constraint. This object is set up automatically
when using the contact action [ContactAction](/ContactAction.md)

For supported quasistatic local-basis mortar contact, Jacobian-bearing evaluations include
displacement derivatives of the averaged secondary nodal normals by default. Weighted-gap and
residual values remain unchanged, and residual-only mode stores no AD derivatives. See
[ComputeWeightedGapLMMechanicalContact](/ComputeWeightedGapLMMechanicalContact.md) for the
supported contact paths, element types, one-ring sparsity and AD storage considerations, and
geometric terms whose derivatives remain frozen.

!syntax parameters /UserObjects/LMWeightedGapUserObject

!syntax inputs /UserObjects/LMWeightedGapUserObject

!syntax children /UserObjects/LMWeightedGapUserObject
