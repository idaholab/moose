# PostprocessorConvergence

This [Convergence](Convergence/index.md) derives from [/IterationCountConvergence.md]
and compares a [Postprocessor](Postprocessors/index.md) value $y$ to a tolerance $\tau$:

!equation
|y| \leq \tau \,.

For this to work as expected, the `execute_on` parameter of the post-processor
must include values that trigger execution before the desired check. For example, for assessing convergence of the  nonlinear solve, the value `NONLINEAR_CONVERGENCE` should be used.

Typically the post-processor used should attempt to approximate the error in a system,
such as [/AverageVariableChange.md].

!syntax parameters /Convergence/PostprocessorConvergence

!syntax inputs /Convergence/PostprocessorConvergence

!syntax children /Convergence/PostprocessorConvergence
