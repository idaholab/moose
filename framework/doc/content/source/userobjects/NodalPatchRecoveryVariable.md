# NodalPatchRecoveryVariable

This user object performs the calculations and parallel communication required to carry out the [Zienkiewicz–Zhu patch recovery](https://doi.org/10.1002/nme.1620330702) for a scalar component of a provided variable.

For recovering a scalar component of a material property, see the related user object: [NodalPatchRecoveryMaterialProperty](NodalPatchRecoveryMaterialProperty.md).

This object can operate on both Variables and AuxVariables, and supports both nodal and elemental types.

!syntax description /UserObjects/NodalPatchRecoveryVariable

!syntax parameters /UserObjects/NodalPatchRecoveryVariable

!syntax inputs /UserObjects/NodalPatchRecoveryVariable
