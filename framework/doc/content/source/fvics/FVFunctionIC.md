# FVFunctionIC

!syntax description /FVICs/FVFunctionIC

Sets an initial condition via a [Function](syntax/Functions/index.md) described by parameter [!param](/FVICs/FVFunctionIC/function). It can be restricted to particular blocks using the [!param](/FVICs/FVFunctionIC/block) parameter.

## Example input syntax

In this example, we set the initial value of variable `u` using a [MooseParsedFunction.md].

!listing test/tests/fvics/function_ic/parsed_function.i block=FVICs Functions

!syntax parameters /FVICs/FVFunctionIC

!syntax inputs /FVICs/FVFunctionIC

!syntax children /FVICs/FVFunctionIC
