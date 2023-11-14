# ProjectedStatefulMaterialNodalPatchRecovery

This user object is derived from [NodalPatchRecoveryMaterialProperty](NodalPatchRecoveryMaterialProperty.md) and performs patch recovery for a material property component.

At step zero this object will compute the material QP values by explicitly calling `initStatefulProperties` in order to project the state that will correspond to the *old state* in the first timestep.

This object is set up by the [ProjectedStatefulMaterialStorageAction](ProjectedStatefulMaterialStorageAction.md).

!syntax description /UserObjects/NodalPatchRecoveryMaterialProperty

!syntax parameters /UserObjects/NodalPatchRecoveryMaterialProperty

!syntax inputs /UserObjects/NodalPatchRecoveryMaterialProperty

!syntax children /UserObjects/NodalPatchRecoveryMaterialProperty
