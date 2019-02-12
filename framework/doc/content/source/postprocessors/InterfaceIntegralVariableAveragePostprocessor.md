# InterfaceIntegralVariableAveragePostprocessor

## IMPORTANT NOTE
*** Any derived class of InterfaceUserObject does not support yet getMaterialProperty, please use auxvariables ***

## Description
!syntax description /Postprocessors/InterfaceIntegralVariableAveragePostprocessor

This post-processor is used to compute an average value across an interface. The type of average to compute is selected by the `average_type` parameter. If the parameter `average_type` is omitted it default on computing the average value between the two input variables across the interface.
The kind of average value are the one available in  [InterfaceAverageTools](/InterfaceAverageTools.md).


## Example Input File Syntax

listing test/tests/postprocessors/interface_average_value/interface_average_value_postprocessor.i block=Postprocessors/diffusivity_average

!syntax parameters /Postprocessors/diffusivity_average

!syntax inputs /Postprocessors/diffusivity_average

!syntax children /Postprocessors/diffusivity_average
