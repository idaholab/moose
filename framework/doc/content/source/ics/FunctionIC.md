# FunctionIC

!syntax description /ICs/FunctionIC

Setting the initial condition using a function may be done for the whole variable domain, or restricted to particular blocks or boundaries.

## Example input syntax

In this example, we set the initial value of variable `u` using a [MooseParsedFunction.md]. This particular example shows that information about the gradient of the parsed function is kept in initial condition, using further mesh refinement.

!listing test/tests/ics/function_ic/parsed_function.i block=ICs Functions

!syntax parameters /ICs/FunctionIC

!syntax inputs /ICs/FunctionIC

!syntax children /ICs/FunctionIC
