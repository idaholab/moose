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
- `COMPUTE`: The norm is actually computed at the time of execution. This is recommended for MultiApp fixed point convergence checks using [Convergence](Convergence/index.md) objects that use post-processor values, such as [PostprocessorConvergence.md]. In this case, if executing MultiApps only on `TIMESTEP_END`, use `execute_on = MULTIAPP_FIXED_POINT_CONVERGENCE`. If executing MultiApps only on `TIMESTEP_BEGIN`, use `execute_on = TIMESTEP_BEGIN`. If executing MultiApps on both `TIMESTEP_BEGIN` and `TIMESTEP_END`, use two instances of this class: one with `execute_on = MULTIAPP_FIXED_POINT_CONVERGENCE` for the `TIMESTEP_END` MultiApps and one with `execute_on = TIMESTEP_BEGIN` for the `TIMESTEP_BEGIN` MultiApps. Then combine both with the `&` operator in [ParsedConvergence.md].

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
