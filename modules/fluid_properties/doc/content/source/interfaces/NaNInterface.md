# NaNInterface

For some objects it is desirable to continue running despite generation of
NaN(s). This class provides an interface for choosing whether to:

- throw an error
- emit a warning, visible on the console for every occurrence of the condition
- throw an exception which will cause the solve to fail for [Steady.md] solves, or a time step reduction
  for [Transient.md] solves
- do nothing at all, just using a quiet NaN which may affect the solution downstream

This interface is commonly used in fluid properties for handling the occurrence of NaN(s).
This interface is not a substitute for implementing domain of validity checks in each fluid property.
This interface can be complimented by the [SolutionInvalidInterface.md], which can let the solver
re-attempt solves when an invalid condition is detected.
