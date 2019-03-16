# InterfaceIntegralVariableAveragePostprocessor

## IMPORTANT NOTE
*** Any derived class of InterfaceUserObject does not support yet getMaterialProperty, please use auxvariables ***

## Description
!syntax description /Postprocessors/InterfaceIntegralVariableAveragePostprocessor

This post-processor is used to compute an average value across an interface. The type of average to compute is selected by changing the `average_type` parameter. If the parameter `average_type` is omitted it defaults to compute the average value between the two input variables across the interface.
The various types of averages that can be computed are describe in more details in  [InterfaceAverageTools](/InterfaceAverageTools.md).


## Example Input File Syntax

listing test/tests/postprocessors/interface_average_value/interface_average_value_postprocessor.i block=Postprocessors/diffusivity_average

!syntax parameters /Postprocessors/InterfaceIntegralVariableAveragePostprocessor

!syntax inputs /Postprocessors/InterfaceIntegralVariableAveragePostprocessor

!syntax children /Postprocessors/InterfaceIntegralVariableAveragePostprocessor
