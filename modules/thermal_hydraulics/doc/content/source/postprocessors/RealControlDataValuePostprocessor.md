# RealControlDataValuePostprocessor

!syntax description /Postprocessors/RealControlDataValuePostprocessor

!alert note
This can only be used with a [THMProblem.md], usually within a THM simulation.

## Example input syntax

In this example, the value of the function is captured by the `T_inlet_fn` [ControlLogic](syntax/ControlLogic/index.md)
and output as a postprocessor using the `T_ctrl` `RealControlDataValuePostprocessor`.

!listing test/tests/controls/get_function_value_control/test.i block=Functions ControlLogic Postprocessors

!syntax parameters /Postprocessors/RealControlDataValuePostprocessor

!syntax inputs /Postprocessors/RealControlDataValuePostprocessor

!syntax children /Postprocessors/RealControlDataValuePostprocessor
