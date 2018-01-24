# RealFunctionControl
The `RealFunctionControl` object is designed to control a "Real" parameter with a function rather
than use the value specified in the input file. This object is mainly a demonstration of how to
create a Control object and modify a parameter.

## Example
Consider a simulation that solves the diffusion equation, where the Laplacian term has a
coefficient, but the coefficient is defined as a constant input parameter ("coef"). For some
reason, it is desired to control this coefficient and replace the constant value with a function
that varies with space and time; this function is defined in the [Functions] block.

The `RealFunctionControl` object is designed for this purpose as shown in \ref{real_func_ex}.

!listing test/tests/controls/real_function_control/real_function_control.i block=Controls id=real_func_ex caption=Control block demonstrating the use of the `RealFunctionControl` object.

Notice that the "parameter" input parameter is expecting a parameter name which can be defined
in various forms.
!include docs/content/documentation/systems/Controls/framework/TimePeriod.md re=For a dis(.*?)section\.

!syntax parameters /Controls/RealFunctionControl

!syntax inputs /Controls/RealFunctionControl

!syntax children /Controls/RealFunctionControl
