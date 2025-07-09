# PostprocessorConvergence

This [Convergence](Convergence/index.md) derives from [/IterationCountConvergence.md]
and compares a [Postprocessor](Postprocessors/index.md) value $y$ to a tolerance $\tau$:

!equation
|y| \leq \tau \,.

For this to work as expected, the `execute_on` parameter of the post-processor
must include values that trigger execution before the desired check. For example, for assessing convergence of the  nonlinear solve, the value `NONLINEAR_CONVERGENCE` should be used. For assessing convergence of a MultiApp fixed point solve, the following recommendations apply:

- If executing MultiApps only on `TIMESTEP_END`, use `execute_on = MULTIAPP_FIXED_POINT_CONVERGENCE`.
- If executing MultiApps only on `TIMESTEP_BEGIN`, use `execute_on = TIMESTEP_BEGIN`.
- If executing MultiApps on both `TIMESTEP_BEGIN` and `TIMESTEP_END`, use two instances of this class: one with `execute_on = MULTIAPP_FIXED_POINT_CONVERGENCE` for the `TIMESTEP_END` MultiApps and one with `execute_on = TIMESTEP_BEGIN` for the `TIMESTEP_BEGIN` MultiApps. Then combine both with the `&` operator in [ParsedConvergence.md].

Typically the post-processor used should attempt to approximate the error in a system,
such as [/AverageVariableChange.md].

!syntax parameters /Convergence/PostprocessorConvergence

!syntax inputs /Convergence/PostprocessorConvergence

!syntax children /Convergence/PostprocessorConvergence
