# Systems overview

There are four types of concrete systems in the MOOSE framework:

- [AuxiliarySystem.md]: This system holds auxiliary variable and degree of
  freedom information
- [DisplacedSystem.md]: This system wraps either nonlinear or auxiliary systems
  within a [displaced problem](DisplacedProblem.md) context
- [NonlinearEigenSystem.md]: This system is used for solving
  [eigen problems](EigenProblem.md) of the form $\mathbf{A}\vec{x} =
  \lambda\mathbf{B}\vec{x}$
  and interfaces with [SLEPc](https://slepc.upv.es/).
- [NonlinearSystem.md]: This system is used for solving nonlinear systems of
  equations and interfaces with [PETSc](https://petsc.org).

Both `NonlinearSystem` and `NonlinearEigenSystem` inherit from [NonlinearSystemBase.md]
which implements systems such as automatic scaling. `NonlinearSystemBase`,
`DisplacedSystem`, and `AuxiliarySystem` all inherit from [SystemBase.md] which
wraps the libMesh `System` object and provides APIs for accessing system vector
and matrix data among other things.
