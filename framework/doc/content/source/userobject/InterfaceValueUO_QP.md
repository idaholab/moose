# InterfaceValueUO_QP

## IMPORTANT NOTE
*** Any derived class of InterfaceUserObject does not support yet getMaterialProperty, please use auxvariables***

## Description
!syntax description /UserObjects/InterfaceValueUO_QP

InterfaceValueUO_QP is a user object computing and storing average values across an interface for each quadrature. The kind of average value are the one available in [InterfaceValueTools](/InterfaceValueTools.md).

The stored value can be converted into an AuxVariable by using [InterfaceValueUserObjectAux](/InterfaceValueUserObjectAux.md) AuxKernel.


## Example Input File Syntax

listing test/tests/userobjects/interface_user_object/interface_user_object_QP.i block=UserObjects/interface_average_uo

!syntax description /UserObjects/InterfaceValueUO_QP

!syntax parameters /UserObjects/InterfaceValueUO_QP

!syntax inputs /UserObjects/InterfaceValueUO_QP

!syntax children /UserObjects/InterfaceValueUO_QP
