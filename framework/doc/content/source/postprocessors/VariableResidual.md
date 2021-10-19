# VariableResidual

!syntax description /Postprocessors/VariableResidual

This postprocessor allows a user to retrieve the residual at different times in the simulation using the `execute_on` parameter.

The residual for the entire system may be found using the [Residual.md] postprocessor.

## Example input syntax

In this example, the residual is output for variable `u` and `v` with two `VariableResidual` postprocessors.

!listing test/tests/postprocessors/variable_residual_norm/variable_residual.i block=Postprocessors

!syntax parameters /Postprocessors/VariableResidual

!syntax inputs /Postprocessors/VariableResidual

!syntax children /Postprocessors/VariableResidual
