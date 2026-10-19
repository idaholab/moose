# LMWeightedGapUserObject

## Description

The `LMWeightedGapUserObject` object provides the Lagrange multiplier and
interpolation function for the enforcement of mortar mechanical constraints.
In essence, the Lagrange multiplier is provided to [NormalMortarMechanicalContact](/NormalMortarMechanicalContact.md) to enforce the non-penetration constraint. This object is set up automatically
when using the contact action [ContactAction](/ContactAction.md)

Setting [!param](/UserObjects/LMWeightedGapUserObject/use_nodal_scaling) to `true` enables the
node-based Lagrange multiplier scaling of [!citep](popp2013improved) to improve the conditioning of
partially covered (edge-dropping) mortar interfaces; see
[ComputeWeightedGapLMMechanicalContact](/ComputeWeightedGapLMMechanicalContact.md) for details and
current limitations.

Setting [!param](/UserObjects/LMWeightedGapUserObject/derive_c_from_elasticity) to `true` derives a
per-node contact stiffness from the elasticity tensors of the two contacting bodies, which the
mortar contact constraints use when `use_derived_c = true`. It is normally set by
[ContactAction](/ContactAction.md) with `c_strategy = physical`, which describes the stiffness and
its use.

!syntax parameters /UserObjects/LMWeightedGapUserObject

!syntax inputs /UserObjects/LMWeightedGapUserObject

!syntax children /UserObjects/LMWeightedGapUserObject
