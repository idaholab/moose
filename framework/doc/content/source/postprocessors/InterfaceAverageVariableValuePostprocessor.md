# InterfaceAverageVariableValuePostprocessor

## Description

!syntax description /Postprocessors/InterfaceAverageVariableValuePostprocessor

This post-processor is used to compute an average value across an interface. The type of average to compute is selected by changing the `interface_value_type` parameter. If the parameter `interface_value_type` is omitted it defaults to compute the average value between the two input variables across the interface.
The various types of averages that can be computed are describe in more details in [InterfaceValueTools](/InterfaceValueTools.md).

## Example Input File Syntax

!listing test/tests/postprocessors/interface_value/interface_average_variable_value_postprocessor.i block=Postprocessors/diffusivity_average

!syntax parameters /Postprocessors/InterfaceAverageVariableValuePostprocessor

!syntax inputs /Postprocessors/InterfaceAverageVariableValuePostprocessor

!syntax children /Postprocessors/InterfaceAverageVariableValuePostprocessor
