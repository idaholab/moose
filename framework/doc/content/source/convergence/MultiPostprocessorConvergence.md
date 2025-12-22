# MultiPostprocessorConvergence

This [Convergence](Convergence/index.md) checks multiple post-processor values against tolerances:

!equation
|y| < \tau_y

where $y$ is the checked post-processor quantity (variable step norm or residual norm), and $\tau_y$ is the associated tolerance.

The object returns `CONVERGED` if all of the checks are true and `ITERATING` otherwise.

The parameter [!param](/Convergence/MultiPostprocessorConvergence/require_one_iteration) is used to specify whether at least one iteration should be required. This is recommended to be set to `true` if any of the post-processors correspond to step criteria, which are always equal to zero for the first iteration. Otherwise, `false` is recommended.

!alert warning title=Divergence criteria
This class has no divergence criteria, so either the derived class must supply some, or it should be used in conjunction with other `Convergence` objects that do provide some.

!syntax parameters /Convergence/MultiPostprocessorConvergence

!syntax inputs /Convergence/MultiPostprocessorConvergence

!syntax children /Convergence/MultiPostprocessorConvergence
