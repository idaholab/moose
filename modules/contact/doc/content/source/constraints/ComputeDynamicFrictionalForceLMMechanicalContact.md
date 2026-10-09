# ComputeDynamicFrictionalForceLMMechanicalContact

!syntax description /Constraints/ComputeDynamicFrictionalForceLMMechanicalContact

When the mortar mechanical contact constraints are used in dynamic simulations, the normal contact constraints need to be stabilized by ensuring that normal gap derivatives are included in the definition. This is a way of guaranteeing the 'persistency' condition, i.e. not only do we enforce the constraints instantaneously, but also that it will remain in contact. This approximate contact constraint stabilization is performed in [ComputeDynamicWeightedGapLMMechanicalContact](/ComputeDynamicWeightedGapLMMechanicalContact.md), from which this object inherits. Therefore `ComputeDynamicFrictionalForceLMMechanicalContact` is used as the base frictional mortar contact class to implement friction models that in a dynamic setting.

When [!param](/Constraints/ComputeDynamicFrictionalForceLMMechanicalContact/function_friction) is provided, the friction coefficient is evaluated at each node with the contact pressure and nodal slip rate of the previous time step. The nodal slip rate is the weighted tangential velocity divided by the integral of the same test function, which is the nodal coefficient of the mortar projection of the tangential velocity [!citep](wohlmuth2011variationally).

The `capture_tolerance` is an optional contact parameter used in dynamic contact constraints to determine when to impose the persistency condition for normal contact. For relevant, general equations, see [!citep](tal2018dynamic).

!syntax parameters /Constraints/ComputeDynamicFrictionalForceLMMechanicalContact

!syntax inputs /Constraints/ComputeDynamicFrictionalForceLMMechanicalContact

!syntax children /Constraints/ComputeDynamicFrictionalForceLMMechanicalContact
