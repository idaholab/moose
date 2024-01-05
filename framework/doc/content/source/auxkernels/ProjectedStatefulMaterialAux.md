# ProjectedStatefulMaterial...Aux

This AuxKernel simply returns the value of a given material property (or component of the property value) at a quadrature point with the purpose of projecting the property onto an elemental basis function (e.g. first order monomial).

Variants include:

- `ProjectedStatefulMaterialRealAux`
- `ProjectedStatefulMaterialRealVectorValueAux`
- `ProjectedStatefulMaterialRankTwoTensorAux`
- `ProjectedStatefulMaterialRankFourTensorAux`
- `ADProjectedStatefulMaterialRealAux`
- `ADProjectedStatefulMaterialRealVectorValueAux`
- `ADProjectedStatefulMaterialRankTwoTensorAux`
- `ADProjectedStatefulMaterialRankFourTensorAux`

At step zero this object will compute the material QP values by explicitly calling `initStatefulProperties` in order to project the state that will correspond to the *old state* in the first timestep.

This object is set up by the [ProjectedStatefulMaterialStorageAction](ProjectedStatefulMaterialStorageAction.md).

See also [ProjectedMaterialPropertyNodalPatchRecoveryAux](ProjectedMaterialPropertyNodalPatchRecoveryAux.md) for the AuxKernel that projects onto nodal basis functions.

!syntax description /AuxKernels/ProjectedStatefulMaterialRealAux

!syntax parameters /AuxKernels/ProjectedStatefulMaterialRealAux

!syntax inputs /AuxKernels/ProjectedStatefulMaterialRealAux

!syntax children /AuxKernels/ProjectedStatefulMaterialRealAux
