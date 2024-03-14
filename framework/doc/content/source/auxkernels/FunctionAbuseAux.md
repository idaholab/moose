# FunctionAbuseAux

!syntax description /AuxKernels/FunctionAbuseAux

The `FunctionAbuseAux` lets users pass variables, functions, postprocessors and other [Functors](Functors/index.md) as argument to a variable. It is generally useful when the function shape is only available as a [Function](Functions/index.md), even
though it would often be more appropriate as a material property, hence the `abuse` qualification.

!alert warning
If any of the arguments are nonlinear variables or depend on nonlinear variables, you should not
use the `FunctionAbuseAux` when also using [automatic differentiation](automatic_differentiation/index.md) and Newton's method.
Auxiliary variables cannot store derivatives with regards to the nonlinear variables, so the Jacobian for Newton's method
would be incomplete.

!alert warning
If using auxiliary variables as arguments to the `FunctionAbuseAux`, you must check that the ordering of execution
is as expected, in that the auxiliary kernels that set those auxiliary variables are executed BEFORE the `FunctionAbuseAux`.
This can be done with the [!param](/Debug/show_execution_order) `Debug` parameter.

## Example syntax

In this example, the `FunctionAbuseAux` to evaluate the simple correlation written with
a time `t` dependency in `Function` density_function. The temperature, an auxiliary variable,
is passed as the time functor.

!listing test/tests/auxkernels/function_abuse_aux/test.i block=Functions AuxKernels

!syntax parameters /AuxKernels/FunctionAbuseAux

!syntax inputs /AuxKernels/FunctionAbuseAux

!syntax children /AuxKernels/FunctionAbuseAux
