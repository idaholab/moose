# PostprocessorConvergence

This [Convergence](Convergence/index.md) derives from [/IterationCountConvergence.md]
and compares a [Postprocessor](Postprocessors/index.md) value $y$ to a tolerance $\tau$:

!equation
|y| \leq \tau \,.

For this to work as expected, the `execute_on` parameter of the post-processor
must include values that trigger execution before the desired check. For example, for assessing convergence of the  nonlinear solve, the value `NONLINEAR_CONVERGENCE` should be used. For assessing convergence of a MultiApp fixed point solve, the appropriate `execute_on` depends on when the MultiApps are executed and on the post-processor type. For example, for [/Residual.md], `TIMESTEP_BEGIN` is appropriate for MultiApps executing on `TIMESTEP_BEGIN`, and `MULTIAPP_FIXED_POINT_CONVERGENCE` is appropriate for MultiApps executing on `TIMESTEP_END`. See [SetupInterface.md] for details on different execution points.

Typically the post-processor used should attempt to approximate the error in a system,
such as [/AverageVariableChange.md].

The parameter [!param](/Convergence/PostprocessorConvergence/max_diverging_iterations) may be used to diverge after the specified number of consecutive iterations for which the post-processor/error value is "diverging". By default, "diverging" means the error value is getting larger, but the parameter [!param](/Convergence/PostprocessorConvergence/diverging_iteration_rel_reduction) can be used to specify some minimum reduction value $\tau_\text{reduction,min}$ such that the "diverging" condition is the following:

!equation
\frac{|y_{\ell-1}| - |y_\ell|}{|y_{\ell-1}|} < \tau_\text{reduction,min}

This can be used to terminate iteration when convergence is proceeding too slowly.

!syntax parameters /Convergence/PostprocessorConvergence

!syntax inputs /Convergence/PostprocessorConvergence

!syntax children /Convergence/PostprocessorConvergence
