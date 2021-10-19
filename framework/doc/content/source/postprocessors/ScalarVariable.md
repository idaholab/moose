# ScalarVariable

!syntax description /Postprocessors/ScalarVariable

## Example input syntax

In this example, the scalar variable `v` is the solution of a simple reaction ODE.
We use the `ScalarVariable` postprocessor to output the value of the scalar variable
to a CSV file.

!listing test/tests/postprocessors/scalar_variable/scalar_variable_pps.i block=Postprocessors/reporter

!syntax parameters /Postprocessors/ScalarVariable

!syntax inputs /Postprocessors/ScalarVariable

!syntax children /Postprocessors/ScalarVariable
