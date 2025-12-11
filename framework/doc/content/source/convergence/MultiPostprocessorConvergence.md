# MultiPostprocessorConvergence

This [Convergence](Convergence/index.md) derives from [/IterationCountConvergence.md] and checks multiple post-processor values against tolerances:

!equation
|y| < \tau_y

where $y$ is the checked post-processor quantity (variable step norm or residual norm), and $\tau_y$ is the associated tolerance.

The object returns `CONVERGED` if all of the checks are true and `ITERATING` otherwise.

For this to work as expected, the `execute_on` parameter of the post-processors
must include values that trigger execution before the desired check. For example, for assessing convergence of the  nonlinear solve, the value `NONLINEAR_CONVERGENCE` should be used. For assessing convergence of a MultiApp fixed point solve, the appropriate `execute_on` depends on when the MultiApps are executed and on the post-processor type. For example, for [/Residual.md], `TIMESTEP_BEGIN` is appropriate for MultiApps executing on `TIMESTEP_BEGIN`, and `MULTIAPP_FIXED_POINT_CONVERGENCE` is appropriate for MultiApps executing on `TIMESTEP_END`. See [SetupInterface.md] for details on different execution points.

!alert note
The parameter [!param](/Convergence/MultiPostprocessorConvergence/min_iterations) is recommended to be set $\geq 1$ whenever step criteria are used, since the step in the first iteration is always zero.

!syntax parameters /Convergence/MultiPostprocessorConvergence

!syntax inputs /Convergence/MultiPostprocessorConvergence

!syntax children /Convergence/MultiPostprocessorConvergence
