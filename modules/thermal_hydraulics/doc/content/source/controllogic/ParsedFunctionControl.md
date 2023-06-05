# ParsedFunctionControl

The ParsedFunctionControl class provides the ability to take a formula as a parameter and evaluate
it as a function.  The quantities that can be used are (a) functions, (b) post-processors, (c) scalar
variables, (d) real-valued control data, and (e) bool-valued control data.

The functions are evaluated at the current simulation time and at the (0,0,0) point.

!alert note
Control data is only declared by ControlLogic in the thermal hydraulics module. To use the `ParsedFunctionControl`
in other applications, do not specify any parameters for the control data.

!syntax parameters /ControlLogic/ParsedFunctionControl

!syntax inputs /ControlLogic/ParsedFunctionControl

!syntax children /ControlLogic/ParsedFunctionControl
