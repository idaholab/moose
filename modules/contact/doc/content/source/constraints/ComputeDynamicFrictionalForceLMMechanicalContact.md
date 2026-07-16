# ComputeDynamicFrictionalForceLMMechanicalContact

!syntax description /Constraints/ComputeDynamicFrictionalForceLMMechanicalContact

When the mortar mechanical contact constraints are used in dynamic simulations, the normal contact constraints need to be stabilized by ensuring that normal gap derivatives are included in the definition. This is a way of guaranteeing the 'persistency' condition, i.e. not only do we enforce the constraints instantaneously, but also that it will remain in contact. This approximate contact constraint stabilization is performed in [ComputeDynamicWeightedGapLMMechanicalContact](/ComputeDynamicWeightedGapLMMechanicalContact.md), from which this object inherits. Therefore `ComputeDynamicFrictionalForceLMMechanicalContact` is used as the base frictional mortar contact class to implement friction models that in a dynamic setting.

The `capture_tolerance` is an optional contact parameter used in dynamic contact constraints to determine when to impose the persistency condition for normal contact. For relevant, general equations, see [!citep](tal2018dynamic).

The Coulomb friction bound can be regularized with
[!param](/Constraints/ComputeDynamicFrictionalForceLMMechanicalContact/friction_elastic_slip) and
[!param](/Constraints/ComputeDynamicFrictionalForceLMMechanicalContact/friction_coefficient_regularization).
The elastic-slip parameter adds a small tangential compliance before the full Coulomb bound is
reached. With `ARCTAN_SLIP`,
[!param](/Constraints/ComputeDynamicFrictionalForceLMMechanicalContact/friction_reference_slip) is a
reference slip distance and is compared against the current step slip increment. If
[!param](/Constraints/ComputeDynamicFrictionalForceLMMechanicalContact/function_friction) is used, that
friction function is still evaluated with the previous-step real tangential velocity when available.
Users should check timestep sensitivity when enabling this slip-increment regularization.
The dynamic mortar Lagrange multiplier contact objects do not support recover/restart of their contact
history data.

!syntax parameters /Constraints/ComputeDynamicFrictionalForceLMMechanicalContact

!syntax inputs /Constraints/ComputeDynamicFrictionalForceLMMechanicalContact

!syntax children /Constraints/ComputeDynamicFrictionalForceLMMechanicalContact
