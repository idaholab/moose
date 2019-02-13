# InterfaceUOPPS

## IMPORTANT NOTE
*** Any derived class of InterfaceUserObject does not support yet getMaterialProperty, please use auxvariables ***

## Description
!syntax description /Postprocessors/InterfaceUOPPS

InterfaceUOPPS is a postprocessor for [InterfaceUO](/InterfaceUO.md).
It create a global variable from the value computed in the linked userobject.

## Example Input File Syntax

listing test/tests/userobjects/interface_user_object/interface_user_object.i block=Postprocessors/interface_average_PP
