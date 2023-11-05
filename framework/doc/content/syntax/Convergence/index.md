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


### +Solver convergence criteria parameters+

Parameters for setting absolute convergence, relative convergence etc. are usually set in the `Executioner` block. However, now they can also prescribed in the `Convergence` block.

We shall provide a few guidelines for setting convergence parameters.

#### 1. Absolute and relative tolerances

- +Absolute Tolerance (`abs_tol`)+: This is a parameter that determines the threshold at which the residual norm of the solution is deemed small enough in absolute terms.
- +Relative Tolerance (`rel_tol`)+: This parameter sets the threshold for the residual norm in relation to the norm of the right-hand side of the equation.


Considering that nonlinear systems are ultimately solved via linearization the user should append `l_` for linear systems, or `nl_` for nonlinear ones, on a per case bases.

#### 2. Choosing appropiate `abs_tol` and `rel_tol`

- To start, apply the same values for both `abs_tol` and `rel_tol` in both preconditioned and non-preconditioned systems. This allows for a straightforward comparison under similar stopping criteria.
- +Preconditioning+ aims to improve the numerical properties of an iterative solver (specifically the condition number). This often results in the preconditioned system converging more quickly or requiring fewer iterations. However, preconditioning modifies the scale and the properties of the problem, which can affect the scale of the residuals.
- The user must make sure that `abs_tol` and `rel_tol` are set in a manner that mirrors the +scaling+ of the problem. If preconditioning significantly changes the scale (which is often the case), the user needs to adjust these tolerances to accommodate the changes.
- The user should +experiment+ with different settings of `abs_tol` and `rel_tol` to observe their effect on convergence and the quality of the solution. To make a fair comparison, one should ensure that the solver’s behavior (in terms of convergence and accuracy) is similar under both settings.

#### Other stopping criteria

Presuming the user gathered enough knowledge about the solver behaviour and the nature of the problem, addtional stopping requirements can be added, such as `nl_max_its` which instructs after how many iterations to abort the solver, or `nl_abs_step_tol`, which indicates what tolerance to be accpeted at each solver step. 

## Example input syntax

!listing test/tests/convergence/diffusion_convergence.i block=Convergence

where the [!param](/Convergence/type) indicates the convergence type to be used, which currently can be `ResidualConvergence` and `ReferenceResidualConvergence`.

