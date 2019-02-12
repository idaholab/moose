# InterfaceUO

## IMPORTANT NOTE
*** Any derived class of InterfaceUserObject does not support yet getMaterialProperty, please use auxvariables***

## Description
!syntax description /UserObjects/InterfaceUO

InterfaceUO is a user object computing an average value across an interface. The kind of average value are the one available in  [InterfaceAverageTools](/InterfaceAverageTools.md).

## Example Input File Syntax

listing test/tests/userobjects/interface_user_object/interface_user_object.i block=UserObjects/interface_average

!syntax parameters /UserObjects/interface_average

!syntax inputs /UserObjects/interface_average

!syntax children /UserObjects/interface_average
