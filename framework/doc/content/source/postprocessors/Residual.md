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
  value; for example, executing on `NONLINEAR` yields 0.
- `COMPUTE`: The norm is actually computed at the time of execution.

A common use of this post-processor is to assess the convergence of iterative solve loops
using [PostprocessorConvergence.md] or [MultiPostprocessorConvergence.md], for example.
For this to work as expected, the `execute_on` parameter of the post-processor
must include values that trigger execution before the desired check. The following table
lists general recommendations for which `execute_on` flags to include for each type of
convergence check. See [SetupInterface.md] for details on different execution points.

| Convergence Check | Recommended `residual_type` | Recommended `execute_on` |
| :- | :- | :- |
| Nonlinear | `CURRENT` | `NONLINEAR_CONVERGENCE` |
| MultiApp fixed point, executing MultiApps only on `TIMESTEP_BEGIN` | `COMPUTE` | `TIMESTEP_BEGIN` |
| MultiApp fixed point, executing MultiApps only on `TIMESTEP_END` | `COMPUTE` | `MULTIAPP_FIXED_POINT_CONVERGENCE` |
| MultiApp fixed point, executing MultiApps only on both `TIMESTEP_BEGIN` and `TIMESTEP_END` | `COMPUTE` | Use two `Residual` objects, one with `execute_on = MULTIAPP_FIXED_POINT_CONVERGENCE` for the `TIMESTEP_END` MultiApps and one with `execute_on = TIMESTEP_BEGIN` for the `TIMESTEP_BEGIN` MultiApps. Then combine both with the `&` operator in [ParsedConvergence.md] |
| Multi-system fixed point | `COMPUTE` | `MULTISYSTEM_FIXED_POINT_ITERATION_END` |

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
