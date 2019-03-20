# InterfaceValueUOPPS

## IMPORTANT NOTE
*** Any derived class of InterfaceUserObject does not support yet getMaterialProperty, please use auxvariables ***

## Description
!syntax description /Postprocessors/InterfaceValueUOPPS

InterfaceValueUOPPS is a postprocessor for [InterfaceValueAverageUO](/InterfaceValueAverageUO.md).
It creates a global variable from the value computed in the linked userobject.

## Example Input File Syntax

listing test/tests/userobjects/interface_user_object/interface_user_object.i block=Postprocessors/interface_average_PP
