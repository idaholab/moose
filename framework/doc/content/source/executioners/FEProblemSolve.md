# FEProblemSolve

The `FEProblemSolve` class has two main roles:

- handle a variety of parameters for the [linear and nonlinear solves](systems/NonlinearSystem.md)
- encapsulate the solve function call with [geometric grid sequencing calls](syntax/Executioner/index.md#Grid Sequencing) for nonlinear problems

The `FEProblemSolve` is a solve executioner nested inside most executioners,
such as [Steady](executioners/Steady.md) and [Transient](executioners/Transient.md) but notably *not* in the [Eigenvalue](executioners/Eigenvalue.md) executioner.