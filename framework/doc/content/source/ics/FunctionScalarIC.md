# FunctionScalarIC

!syntax description /ICs/FunctionScalarIC

The function is evaluated at the simulation start time and at the point of coordinate (0, 0, 0).

## Example input syntax

In this example, the scalar variable `n`, solution of a first order ODE, is initialized using function `f`. Since the start time of the simulation is 0, `f` is evaluated to `cos(0)=1`.

!listing test/tests/ics/function_scalar_ic/function_scalar_ic.i block=ICs Functions

!syntax parameters /ICs/FunctionScalarIC

!syntax inputs /ICs/FunctionScalarIC

!syntax children /ICs/FunctionScalarIC
