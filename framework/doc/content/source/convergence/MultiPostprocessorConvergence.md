# MultiPostprocessorConvergence

This [Convergence](Convergence/index.md) derives from [/IterationCountConvergence.md] and checks multiple post-processor values against tolerances:

!equation
|y| < \tau_y

where $y$ is the checked post-processor quantity, and $\tau_y$ is the associated tolerance.

The object returns `CONVERGED` if all of the checks are true and `ITERATING` otherwise.

For this to work as expected, the `execute_on` parameter of the post-processors
must include values that trigger execution before the desired check. The following table
lists general recommendations for which `execute_on` flags to include for each type of
convergence check. See [SetupInterface.md] for details on different execution points.

| Convergence Check | Recommended `execute_on` |
| :- | :- |
| Nonlinear | `NONLINEAR_CONVERGENCE` |
| Steady-state | `TIMESTEP_END` |
| MultiApp fixed point | `MULTIAPP_FIXED_POINT_CONVERGENCE` (see [/Residual.md] for specific guidance for `Residual`) |
| Multi-system fixed point | `MULTISYSTEM_FIXED_POINT_ITERATION_END` |

!alert note
The parameter [!param](/Convergence/MultiPostprocessorConvergence/min_iterations) is recommended to be set $\geq 1$ whenever step criteria are used, since the step in the first iteration is always zero.

!syntax parameters /Convergence/MultiPostprocessorConvergence

!syntax inputs /Convergence/MultiPostprocessorConvergence

!syntax children /Convergence/MultiPostprocessorConvergence
