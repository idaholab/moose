# PostprocessorSteadyStateConvergence

This class is similar to [/PostprocessorConvergence.md] but does not derive from [/IterationCountConvergence.md]; it compares a [Postprocessor](Postprocessors/index.md) value $y$ to a tolerance $\tau$:

!equation
|y| \leq \tau \,.

The initial steady-state check is skipped to avoid false convergence, due to post-processors falsely computing time derivatives as zero at the initial time.

!syntax parameters /Convergence/PostprocessorSteadyStateConvergence

!syntax inputs /Convergence/PostprocessorSteadyStateConvergence

!syntax children /Convergence/PostprocessorSteadyStateConvergence
