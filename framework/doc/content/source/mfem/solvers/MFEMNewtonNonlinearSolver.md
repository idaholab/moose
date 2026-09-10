# MFEMNewtonNonlinearSolver

!if! function=hasCapability('mfem')

## Overview

Defines and builds an `mfem::NewtonSolver` to solve nonlinear MFEM equation systems.

This solver requires Jacobian information from the MFEM operator and uses the externally configured
MFEM linear solver for the inner linear solves.

## Damping and line search

By default the full Newton update is applied at each nonlinear iteration. Setting
`damping_factor` to a value below one applies that constant fraction of every update instead,
which can keep an update from leaving the region in which the residual is well-behaved at the
cost of a slower, linear convergence rate.

Setting `line_search = backtracking` enables a backtracking line search, which shortens the
update only where needed. Starting from the trial step $t$ given by `damping_factor`, the update
$c$ at the iterate $x$ is accepted if it sufficiently reduces the residual norm,

!equation
\|F(x - t c) - b\| \leq (1 - \alpha t) \|F(x) - b\|,

where $\alpha$ is `line_search_sufficient_decrease`. If it does not, the trial step is
multiplied by `line_search_contraction_factor` and tested again, for at most
`line_search_max_its` trial steps. Each trial step costs one residual evaluation. If no trial
step satisfies the condition the nonlinear solve is terminated and reported as not converged.

Define this object in the [`Solvers`](syntax/Solvers/index.md) block.

!syntax parameters /Solvers/MFEMNewtonNonlinearSolver

!syntax inputs /Solvers/MFEMNewtonNonlinearSolver

!syntax children /Solvers/MFEMNewtonNonlinearSolver

!if-end!

!else
!include mfem/mfem_warning.md
