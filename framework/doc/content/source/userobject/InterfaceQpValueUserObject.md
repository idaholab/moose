# InterfaceQpValueUserObject

## IMPORTANT NOTES
*** Any derived class of InterfaceUserObject does not support yet getMaterialProperty, please use auxvariables***

*** This userobejct only support multiprocessing. Threading is not supported at this time  ***


## Description
!syntax description /UserObjects/InterfaceQpValueUserObject

InterfaceQpValueUserObject is a user object computing and storing average values across an interface for each quadrature. The kind of average value are the one available in [InterfaceValueTools](/InterfaceValueTools.md).

The stored value can be converted into an AuxVariable by using [InterfaceValueUserObjectAux](/InterfaceValueUserObjectAux.md) AuxKernel.


## Example Input File Syntax

listing test/tests/userobjects/interface_user_object/interface_value_user_object_QP.i block=UserObjects/interface_value_uo

!syntax description /UserObjects/InterfaceQpValueUserObject

!syntax parameters /UserObjects/InterfaceQpValueUserObject

!syntax inputs /UserObjects/InterfaceQpValueUserObject

!syntax children /UserObjects/InterfaceQpValueUserObject
