# GlobalStrainUserObject

!syntax description /UserObjects/GlobalStrainUserObject

## Description

`GlobalStrainUserObject` calculates the residual and the jacobian values corresponding to the integral of the stress tensor. In this case, the stress is generated due to global strain or any applied stress on the whole simulation domain.

`ScalarKernel` [GlobalStrain](/GlobalStrain.md) extracts the residual and jacobian values from this `UserObject`.

## Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/global_strain/global_strain.i block=UserObjects/global_strain_uo

!syntax parameters /UserObjects/GlobalStrainUserObject

!syntax inputs /UserObjects/GlobalStrainUserObject

!syntax children /UserObjects/GlobalStrainUserObject
