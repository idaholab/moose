# Residual

!syntax description /Postprocessors/Residual

The nonlinear residual may be queried before a nonlinear iteration or after.
More information about residuals may be found [here](resres).

The residual may be split by variables using the [VariableResidual.md] postprocessor.

## Example input syntax

In this example, `u` is the solution of a diffusion problem. A predictor time integrating
scheme is used and the `Residual` postprocessor reports the residual.

!listing test/tests/predictors/simple/predictor_test.i block=Postprocessors

!syntax parameters /Postprocessors/Residual

!syntax inputs /Postprocessors/Residual

!syntax children /Postprocessors/Residual
