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

## Adaptive linear tolerance

Far from the solution, solving each linear system to a tight tolerance gains little, because
the Newton update is only as good as the linearization. Setting
[!param](/Solvers/MFEMNewtonNonlinearSolver/adaptive_l_tol) to `eisenstat_walker_1` or
`eisenstat_walker_2` instead sets the relative tolerance $\eta_k$ of the linear solve on each
Newton iteration $k$ from the progress of the nonlinear solve, following choices 1 and 2 of
[!cite](eisenstat1996choosing). The first iteration uses
[!param](/Solvers/MFEMNewtonNonlinearSolver/adaptive_l_tol_initial), and later ones use

!equation
\eta_k = \gamma \frac{\left| \|F(x_k)\| - \|F(x_{k-1}) + J(x_{k-1}) s_{k-1}\| \right|}{\|F(x_{k-1})\|}
\quad \text{or} \quad
\eta_k = \gamma \left( \frac{\|F(x_k)\|}{\|F(x_{k-1})\|} \right)^\alpha

for the two choices, where $s_{k-1}$ is the previous update, $\gamma$ is
[!param](/Solvers/MFEMNewtonNonlinearSolver/adaptive_l_tol_gamma) and $\alpha$ is
[!param](/Solvers/MFEMNewtonNonlinearSolver/adaptive_l_tol_alpha). When
$\gamma \eta_{k-1}^\alpha > 0.1$, $\eta_k$ is raised to at least that value, so that the
tolerance does not drop abruptly, and it is never larger than
[!param](/Solvers/MFEMNewtonNonlinearSolver/adaptive_l_tol_max).

The adapted tolerance replaces the relative tolerance of the linear solver, which must therefore be
one of MFEM's own iterative solvers, such as [MFEMCGSolver](MFEMCGSolver.md) or
[MFEMGMRESSolver](MFEMGMRESSolver.md), rather than a hypre solver, and without low-order refinement.

Define this object in the [`Solvers`](syntax/Solvers/index.md) block.

!syntax parameters /Solvers/MFEMNewtonNonlinearSolver

!syntax inputs /Solvers/MFEMNewtonNonlinearSolver

!syntax children /Solvers/MFEMNewtonNonlinearSolver

!if-end!

!else
!include mfem/mfem_warning.md
