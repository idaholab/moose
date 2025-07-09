# Residual

This post-processor computes the nonlinear residual norm.
The parameter [!param](/Postprocessors/Residual/residual_type) may have one of
the following values:

- `FINAL`: The final norm, obtained by `NonlinearSystemBase::finalNonlinearResidual()`
- `INITIAL`: The post-SMO initial residual norm, obtained by `NonlinearSystemBase::preSMOResidual()`.
  For the definition of pre- vs. post-SMO residual, see [NonlinearSystem.md].
- `PRE_SMO`: The pre-SMO initial residual norm, obtained by `NonlinearSystemBase::initialResidual()`
- `CURRENT`: The current norm, obtained directly from the PETSc SNES object.
  Note that the norm obtained from PETSc does not always provide the latest
  value; for example, executing on `NONLINEAR` yields 0. This is recommended
  only for use in nonlinear convergence checks using [Convergence](Convergence/index.md)
  objects that use post-processor values, such as [PostprocessorConvergence.md].
  In this case, it is recommended to set [!param](/Postprocessors/Residual/execute_on) to
  `NONLINEAR_CONVERGENCE` to get the desired behavior for this option.
- `COMPUTE`: The norm is actually computed at the time of execution. This is
  recommended for MultiApp fixed point convergence checks using [Convergence](Convergence/index.md)
  objects that use post-processor values, such as [PostprocessorConvergence.md] (see for recommended [!param](/Postprocessors/Residual/execute_on) value).

More information about residuals and their use in Newton's method may be found
in [NonlinearSystem.md].

The residual may be split by variables using the [VariableResidual.md] postprocessor.

## Example input syntax

In this example, `u` is the solution of a diffusion problem. A predictor time integrating
scheme is used and the `Residual` postprocessor reports the residual.

!listing test/tests/predictors/simple/predictor_test.i block=Postprocessors

!syntax parameters /Postprocessors/Residual

!syntax inputs /Postprocessors/Residual

!syntax children /Postprocessors/Residual
