# GlobalStrainPeriodicDirUserObject

!syntax description /UserObjects/GlobalStrainPeriodicDirUserObject

## Description

This UserObject automatically retrieves directions set as periodic via the periodic boundary condition system.

## AD Global Strain Objects

Used with the automatic differentiation version of Global Strain.

[ADGlobalStrain](/ADGlobalStrain.md)

[ADComputeGlobalStrain](/ADComputeGlobalStrain.md)

[ADGlobalDisplacementAux](/ADGlobalDisplacementAux.md)

!alert note
The `execute_on` parameter should almost always be set to INITIAL only. As built the `GlobalStrainPeriodicDirUserObject` runs on the first solve and stores the periodic directions for subsequent solves.


!syntax parameters /UserObjects/GlobalStrainPeriodicDirUserObject

!syntax inputs /UserObjects/GlobalStrainPeriodicDirUserObject

!syntax children /UserObjects/GlobalStrainPeriodicDirUserObject
