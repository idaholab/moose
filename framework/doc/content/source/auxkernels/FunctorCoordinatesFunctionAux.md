# FunctorCoordinatesFunctionAux

!syntax description /AuxKernels/FunctorCoordinatesFunctionAux

The `FunctorCoordinatesFunctionAux` lets users pass variables, functions, postprocessors and other [Functors](Functors/index.md) as time/coordinates arguments to a function, in order to set an auxiliary variable. It is generally useful when the function shape is only available as a [Function](Functions/index.md), even
though it would often be more appropriate to use a material property.

!alert warning
If any of the arguments are nonlinear variables or depend on nonlinear variables, you should not
use the `FunctorCoordinatesFunctionAux` when also using [automatic differentiation](automatic_differentiation/index.md) and Newton's method.
Auxiliary variables cannot store derivatives with regards to the nonlinear variables, so the Jacobian for Newton's method
would be incomplete.

!alert warning
If using [auxiliary variables](AuxVariables/index.md) as arguments to the `FunctorCoordinatesFunctionAux`, you must make sure the [AuxKernels](AuxKernels/index.md)
computing these auxiliary variables are executed on an `execute_on` schedule happening before the [!param](/AuxKernels/FunctorCoordinatesFunctionAux/execute_on)
schedule of the `FunctorCoordinatesFunctionAux`. This is because the auxiliary variables are read from the system solution vector, which is only
updated after the execution of all auxiliary kernels on a given `execute_on` schedule. You can check that the ordering of execution of auxiliary
kernels is happening on a previous `execute_on` schedule using the [!param](/Debug/show_execution_order) parameter in the `[Debug]` block.

## Example syntax

In this example, the `FunctorCoordinatesFunctionAux` to evaluate the simple correlation written with
a time `t` dependency in `Function` density_function. The temperature, an auxiliary variable,
is passed as the time functor.

!listing test/tests/auxkernels/functor_coordinates_function_aux/test.i block=Functions AuxKernels

!syntax parameters /AuxKernels/FunctorCoordinatesFunctionAux

!syntax inputs /AuxKernels/FunctorCoordinatesFunctionAux

!syntax children /AuxKernels/FunctorCoordinatesFunctionAux
