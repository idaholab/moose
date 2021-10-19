# ScalarL2Error

!syntax description /Postprocessors/ScalarL2Error

The function to compare the scalar variable against is evaluated at the simulation time
and at the (0,0,0) point. Given that a single number is included in this sum, the
L2 norm is the same as the absolute value.

!alert note
This postprocessor does not compute the L2 error for an array scalar variable! It only
uses the first component of the scalar variable.

## Example input syntax

In this example, we solve an ODE for scalar variable `n`, then compute the convergence
to a known analytic solution to examine the order of convergence of a time scheme.

!listing test/tests/time_integrators/scalar/scalar.i block=Postprocessors

!syntax parameters /Postprocessors/ScalarL2Error

!syntax inputs /Postprocessors/ScalarL2Error

!syntax children /Postprocessors/ScalarL2Error
