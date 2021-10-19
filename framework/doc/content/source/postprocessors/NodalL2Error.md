# NodalL2Error

!syntax description /Postprocessors/NodalL2Error

This is typically used to compare a nodal variable to a known analytical solution.
To compute the error with regards to a variable, instead of a function, you may use
a [ParsedAux.md] to store the difference in an `AuxVariable`, then use the `NodalL2Error`
or [NodalL2Norm.md] postprocessor to compute the norm.

## Example input syntax

In this example, variable `u` is the solution of a diffusion-source problem. We know the
analytical solution of this problem and use the `NodalL2Error` postprocessor to examine
the quality of the numerical solution.

!listing test/tests/auxkernels/time_integration/time_integration.i block=Postprocessors Functions

!syntax parameters /Postprocessors/NodalL2Error

!syntax inputs /Postprocessors/NodalL2Error

!syntax children /Postprocessors/NodalL2Error
