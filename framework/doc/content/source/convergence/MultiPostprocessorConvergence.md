# MultiPostprocessorConvergence

This is a base class for [Convergence](Convergence/index.md) that checks multiple post-processor values against tolerances:

!equation
|y| < \tau_y

where $y$ is the checked post-processor quantity (variable step norm or residual norm), and $\tau_y$ is the associated tolerance. Derived classes are responsible for overriding the `getDescriptionErrorToleranceTuples()` method, which provides the tuples composed of the following:

- quantity description
- quantity value, $y$
- quantity tolerance, $\tau_y$

The object returns `CONVERGED` if all of the checks are true and `ITERATING` otherwise.

Derived classes must also override `getMinimumIterations()`, which provides the number of iterations that must be performed before `CONVERGED` can be returned. For example, when step criteria are used, it is recommeded to take at least one iteration.

!alert warning title=Divergence criteria
This class has no divergence criteria, so either the derived class must supply some, or it should be used in conjunction with other `Convergence` objects that do provide some.
