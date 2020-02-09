# InterfaceQpValueUserObject

## IMPORTANT NOTES
*** This userobejct only support multiprocessing. Threading is not supported at this time  ***


## Description
!syntax description /UserObjects/InterfaceQpValueUserObject

InterfaceQpValueUserObject is a user object computing and storing average variable values or rates across an interface for each quadrature. The kind of average value are the one available in [InterfaceValueTools](/InterfaceValueTools.md).
The rate is computed if `compute_rate=true` is set in the input file. Note that this choice is exclusive, to get both the interface value and interface value rate the user must add two separate user objects in the input file.

The InterfaceQpValueUserObject can provide two types of values to other MOOSE systems:

* a qp value by calling `getQpValue`
* an element side average value by calling `getSideAverageValue`

The stored value can be converted into an AuxVariable by using [InterfaceValueUserObjectAux](/InterfaceValueUserObjectAux.md) AuxKernel.


## Example Input File Syntax

listing test/tests/userobjects/interface_user_object/interface_value_user_object_QP.i block=UserObjects/interface_value_uo

!syntax description /UserObjects/InterfaceQpValueUserObject

!syntax parameters /UserObjects/InterfaceQpValueUserObject

!syntax inputs /UserObjects/InterfaceQpValueUserObject

!syntax children /UserObjects/InterfaceQpValueUserObject
