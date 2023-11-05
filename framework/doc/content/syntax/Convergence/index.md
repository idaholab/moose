# Convergence system

The Convergence system provides the infrastructure for creating MOOSE objects that interact with the 
solvers to give the user more control over the application behaviour. 

## Description

By default, MOOSE checks convergence using relative and absolute criteria. Once the residual drops
below either an absolute tolerance, or the residual divided by the initial residual for the current
time step drops below a relative tolerance, the solution is considered converged. This works well for
many problems, but there are some scenarios that are problematic, where the user may desire 
interaction with the solver at runtime for better control or analysis. 

Currently this object supports algebraic two types of convergence
[/ResidualConvergence.md] and 
[ReferenceResidualConvergence.md].

### +Methods supported+

- [/ResidualConvergence.md] tracks the residual decay across iterations. This MOOSE object interfaces directly with the PETSc callback function at each iteration and allows the user to prescribe additional requirements and tests which can be performed at every iteration step. 

- [ReferenceResidualConvergence.md] uses a user-specified reference vector for convergence checks, instead of the initial residual. This is beneficial when solution variables have different scaling. Convergence is achieved when the $L_2$ norm of each solution variable’s residual is less than either the relative tolerance times the $L_2$ norm of the corresponding reference variable or the absolute tolerance. As this method computes relative convergence differently, the required nonlinear relative tolerance to achieve the same error can vary from the default approach in MOOSE. Users must ensure appropriate tolerances are used.


