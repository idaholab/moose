# ComputeDynamicFrictionalForceLMMechanicalContact

When the mortar mechanical contact constraints are used in dynamic simulations, the normal contact constraints need to be stabilized by ensuring that normal gap derivatives are included in the definition. This is a way of guaranteeing the 'persistency' condition, i.e. not only we enforce the constraints instantaneously, but also that it will remain in contact. This approximate contact constraint stabilization is performed in  [`ComputeDynamicWeightedGapLMMechanicalContact`](/ComputeDynamicWeightedGapLMMechanicalContact.md), from which this object inherits. Therefore `ComputeDynamicFrictionalForceLMMechanicalContact` is used as the base frictional mortar contact class to implement friction models that will work in dynamic modeling.


!syntax description /Constraints/ComputeDynamicFrictionalForceLMMechanicalContact

!syntax parameters /Constraints/ComputeDynamicFrictionalForceLMMechanicalContact

!syntax inputs /Constraints/ComputeDynamicFrictionalForceLMMechanicalContact

!syntax children /Constraints/ComputeDynamicFrictionalForceLMMechanicalContact
