# PostprocessorConvergence

This [Convergence](Convergence/index.md) derives from [/IterationCountConvergence.md]
and compares a [Postprocessor](Postprocessors/index.md) value $y$ to a tolerance $\tau$:

!equation
|y| \leq \tau \,.

For this to work as expected, the `execute_on` parameter of the post-processor
must include values that trigger execution before the desired check. The following table
lists general recommendations for which `execute_on` flags to include for each type of
convergence check. See [SetupInterface.md] for details on different execution points.

| Convergence Check | Recommended `execute_on` |
| :- | :- |
| Nonlinear | `NONLINEAR_CONVERGENCE` |
| Steady-state | `TIMESTEP_END` |
| MultiApp fixed point | `MULTIAPP_FIXED_POINT_CONVERGENCE` (see [/Residual.md] for specific guidance for `Residual`) |
| Multi-system fixed point | `MULTISYSTEM_FIXED_POINT_CONVERGENCE` |

!syntax parameters /Convergence/PostprocessorConvergence

!syntax inputs /Convergence/PostprocessorConvergence

!syntax children /Convergence/PostprocessorConvergence
