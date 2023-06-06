# ParsedFunctionControl

The ParsedFunctionControl class provides the ability to take a formula as a parameter and evaluate
it as a function.  The quantities that can be used are (a) functions, (b) post-processors, (c) scalar
variables, (d) real-valued [ControlData.md], and (e) bool-valued [ControlData.md].

The functions are evaluated at the current simulation time and at the (0,0,0) point.

!alert note
[ControlData.md] is only declared by [ControlLogic](syntax/ControlLogic/index.md) in the thermal hydraulics module.
To use the `ParsedFunctionControl` in other applications, do not specify any parameters for the [ControlData.md].

!syntax parameters /ControlLogic/ParsedFunctionControl

!syntax inputs /ControlLogic/ParsedFunctionControl

!syntax children /ControlLogic/ParsedFunctionControl
