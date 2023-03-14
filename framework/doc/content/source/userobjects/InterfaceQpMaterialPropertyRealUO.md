# InterfaceQpMaterialPropertyRealUO

## IMPORTANT NOTES
*** This userobejct only support multiprocessing. Threading is not supported at this time  ***


## Description
!syntax description /UserObjects/InterfaceQpMaterialPropertyRealUO

InterfaceQpMaterialPropertyRealUO is a user object computing and storing average `real` material property values, rates, or increments across an interface for each quadrature point. The kind of average value are the one available in [InterfaceValueTools](/InterfaceValueTools.md).
The value type stored by this userobject is selected via the `value_type` input parameter. For example to get both the material property value and material property rate the user must add two separate user objects in the input file.

The InterfaceQpMaterialPropertyRealUO can provide two types of values to other MOOSE systems:

- a qp value by calling `getQpValue`
- an element side average value by calling `getSideAverageValue`

The stored value can be converted into an AuxVariable by using [InterfaceValueUserObjectAux](/InterfaceValueUserObjectAux.md) AuxKernel.


## Example Input File Syntax

!listing test/tests/userobjects/interface_user_object/interface_mp_real_user_object_QP.i block=UserObjects

!syntax parameters /UserObjects/InterfaceQpMaterialPropertyRealUO

!syntax inputs /UserObjects/InterfaceQpMaterialPropertyRealUO

!syntax children /UserObjects/InterfaceQpMaterialPropertyRealUO
