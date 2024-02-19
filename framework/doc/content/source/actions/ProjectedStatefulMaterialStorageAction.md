# ProjectedStatefulMaterialStorageAction

## Description

The `ProjectedStatefulMaterialStorageAction` is the main action in the
[ProjectedStatefulMaterialStorage](/ProjectedStatefulMaterialStorage/index.md)
system which sets up all necessary objects to project material property
components onto nodal or elemental basis functions.

Projected and interpolated old state can be enabled using the
`use_interpolated_state` parameter available in the `MaterialPropertyInterface`.

### Objects set up by the action

The action will set up aux variables for each scalar component of the projected properties listed in
[!param](/ProjectedStatefulMaterialStorage/ProjectedStatefulMaterialStorageAction/projected_props).
The variables will be marked as hidden and will not appear in any outputs.

The type of each projected material property will be determined automatically and a corresponding
[InterpolatedStatefulMaterial](InterpolatedStatefulMaterial.md) object will be created to
reconstitute a material property of the same type from the old state of the projected variables.

The [!param](/ProjectedStatefulMaterialStorage/ProjectedStatefulMaterialStorageAction/family)
parameter determines whether nodal or elemental basis functions will be used.

#### Elemental basis functions

A [ProjectedStatefulMaterialAux](/ProjectedStatefulMaterialAux.md) aux kernel of the appropriate
type will be added for each scalar material property component (e.g. vector or
tensor components) to perform an elemental projection of the property component.

#### Nodal basis functions

A [ProjectedStatefulMaterialNodalPatchRecovery](/ProjectedStatefulMaterialNodalPatchRecovery.md)
user object of the appropriate type will be created for each material property to prepare data
required for nodal patch recovery of each scalar material property component (e.g. vector or
tensor components).

A [ProjectedMaterialPropertyNodalPatchRecoveryAux](/ProjectedMaterialPropertyNodalPatchRecoveryAux.md)
aux kernel will be added for each scalar material property component to perform nodal patch recovery
using the data collected in the user object.
