# Steady

!syntax description /Executioner/Steady

## Overview

Steady is a general solver for discrete steady-state nonlinear or linear problem:

!equation id=eq:problem
\mathbf{R}(\mathbf{u}) = \mathbf{0}.

By default the line-search Newton in PETSc is used with the PJFNK (preconditioned Jacobian-free Newton Krylov) method.
At each Newton iteration the executioner solves

!equation id=eq:linear-solve
\mathbf{J}(\mathbf{u}^{i-1}) \delta \mathbf{u}^{i} = \mathbf{R}(\mathbf{u}^{i-1}),

where

!equation
\mathbf{J}(\mathbf{u}^{i-1}) \equiv \mathbf{R}'(\mathbf{u}=\mathbf{u}^{i-1})

is the Jacobian matrix evaluated at $\mathbf{u}^{i-1}$.
Jacobian matrix depends on $\mathbf{u}^{i-1}$ for general nonlinear problems while it is constant for linear problems.
The right hand side is also typically referred to as the residual at $\mathbf{u}^{i-1}$ of [eq:problem].
The Krylov methods are employed for solving the above linear equation, which requires only the evaluation of the matrix-vector product $\mathbf{J}(\mathbf{u}^{i-1}) \mathbf{y}$.
Within the MOOSE framework, the Jacobian-Free Newton Krylov method is used that approximates matrix vector products by the Finite-Difference like approximation:

!equation id=eq:mffd
\mathbf{J}(\mathbf{u}^{i-1}) \mathbf{y} \approx \frac{\mathbf{R}(\mathbf{u}^{i-1} + \epsilon \mathbf{y}) - \mathbf{R}(\mathbf{u}^{i-1})}{\epsilon},

where the scalar value $\epsilon$ is chosen by PETSc automatically to approximate $\mathbf{J}(\mathbf{u}^{i-1}) \mathbf{y}$ accurately for the linear solve.
It is noted that for a linear problem of which $\mathbf{R}(\mathbf{u})$ can be expressed as $\mathbf{A} \mathbf{u}-\mathbf{b}$, where matrix $\mathbf{A}$ is the Jacobian independent on $\mathbf{u}$ and $\mathbf{b}$ is the right-hand-side vector, the right hand side of [eq:mffd] is independent on $\epsilon$.
Section 5.5 of PETSc user's manual on matrix-free methods details the algorithm for choosing the value of $\epsilon$.
It is actually the PETSc option `-mat_mffd_err` controls the $\epsilon$ but not `-snes_mf_err` unless we set `-snes_mf_version` to 2 other than the default 1.
This could be changed in future PETSc updates.

The Krylov methods typically also require an approximation of the actual Jacobian $\mathbf{M}(\mathbf{u}^{i-1}) \approx \mathbf{J}(\mathbf{u}^{i-1})$ for pre-conditioning the Krylov solution at each linear iteration.
Note, the preconditioning matrix is seldom the exact Jacobian $\mathbf{J}$ because it would require too much computational time and memory to compute, and in some cases is simply impossible to compute.
By default the type of Krylov method in use is GMRES because it does not have assumptions on the underlying Jacobian.
The initial guess for each linear solve is always set to zero, which implies that the initial linear residual is the same of the nonlinear residual.
The residual norm at each linear iteration is evaluated by PETSc, for instance, during updating the Hessenberg matrix if GMRES method is used.
At the conclusion of the nonlinear iteration, the solution is updated as follows

!equation id=eq:nonlinear-update
\mathbf{u}^{i} = \mathbf{u}^{i-1} + \alpha \delta \mathbf{u}^{i}

where $\alpha$ is determined by the line-search algorithm.
We can see that at each nonlinear or Newton iteration, we will need to update the preconditioning matrix and evaluate the residual with the updated solution.
At each linear iteration, we simply need a residual evaluation and the operation of the preconditioner built from the preconditioning matrix. In PETSc the preconditioner type refers to the method to obtain an approximation of the inverse of $\mathbf{M}$ and not a means to compute the elements of $\mathbf{M}$.
It is noted that the default preconditioner type depends on the number of processors and also depends on the assembled preconditioning matrix $\mathbf{M}$.
Typically incomplete LU (PCILU) is the default type with one processor and block Jacobi (PCBJACOBI) is the type with multiple processors.
Consequently, you will not see the same convergence with the different number of processors. Note, there are two approximations in play here: (1) the Jacobian $\mathbf{J}$ is approximated by a matrix $\mathbf{M}$ that is easier to compute, and (2) the matrix $\mathbf{M}$ is inverted approximately.
The preconditioning matrix $\mathbf{M}$ can be viewed with the PETSc option `-ksp_view_pmat`.

## Solve Type

The general method in which the nonlinear system is solved is controled by the [!param](/Executioner/Steady/solve_type) parameter. Below is a description of each of the options:

- `PJFNK` is the default solve type. It makes the executioner perform Jacobian-free linear solves at each Newton iteration with the preconditioner built from the preconditioning matrix $\mathbf{M}$. By default, the preconditioning matrix is block-diagonal with each block corresponding to a single MOOSE variable without custom preconditioning, refer to [Preconditioning](/Preconditioning/index.md). Off-diagonal Jacobian terms are ignored. It essentially activates the matrix-free Jacobian-vector products, and the preconditioning matrix.
- `JFNK` means there is no preconditioning during the Krylov solve. No Jacobian will be assembled. It essentially activates the matrix-free Jacobian-vector products and no preconditioning matrix.
- `LINEAR` will use PETSc control parameter `-ksp_only` to set the type of SNES for solving the linear system. Note that it only works when you have an *exact* Jacobian because it is not activating matrix-free calculations.
- `NEWTON` means PETSc will use the Jacobian provided by kernels (typically not exact) to do the Krylov solve. If the Jacobian is not exact, Newton update in [eq:nonlinear-update] will not reduce the residual effectively and typically results into an unconverged Newton iteration.
- `FD` means the Jacobian is assembled via a finite differencing method. This is costly and should used only for testing purpose.

## Preconditioning

- Krylov methods need preconditioning to be efficient (or even effective!).
- Even though the Jacobian is never formed, JFNK methods still require preconditioning.
- MOOSE's automatic (without user intervention) preconditioning is fairly minimal.
- Many options exist for implementing improved preconditioning in MOOSE.

### Preconditioned JFNK

- Using right preconditioning, solve

!equation
\boldsymbol{R}'(\boldsymbol{u}^{i-1}) \boldsymbol{M}^{-1} (\boldsymbol{M} \delta \boldsymbol{u}^{i}) = -\boldsymbol{R}(\boldsymbol{u}^{i-1})

- $\boldsymbol{M}$ symbolically represents the preconditioning matrix or process
- Inside GMRES, we only apply the action of $\boldsymbol{M}^{-1}$ on a vector
- Right preconditioned matrix free version

!equation
\boldsymbol{R}' (\boldsymbol{u}) \boldsymbol{M}^{-1}\boldsymbol{v} \approx \frac{\boldsymbol{R}(\boldsymbol{u} + \epsilon \boldsymbol{M}^{-1}\boldsymbol{v}) - \boldsymbol{R}(\boldsymbol{u})}{\epsilon}

## PETSc Options

PETSc parameters can either be set on the command line or by using the [!param](/Executioner/Steady/petsc_options), [!param](/Executioner/Steady/petsc_options_iname), and [!param](/Executioner/Steady/petsc_options_value) parameters. Several PETSc parameters that users could frequently use are listed below:

!table caption=Common stand-alone petsc options
| `petsc_options` | Description |
| - | - |
| `-snes_ksp_ew` | Variable linear solve tolerance -- useful for transient solves |
| `-help` | Show PETSc options during the solve |


!table caption=Common petsc options and values
| `petsc_options_iname` | `petsc_options_value` | Description |
| - | - | - |
| `-pc_type` | `ilu` | Default for serial |
|   | `bjacobi` | Default for parallel with `-sub_pc_type ilu` |
|   | `asm` | Additive Schwartz with `-sub_pc_type ilu` |
|   | `lu` | Full LU, serial only! |
|   | `gamg` | PETSc Geometric AMG Preconditioner |
|   | `hypre` | Hypre, usually used with `boomeramg` |
| `-sub_pc_type` | `ilu, lu, hypre` | Can be used with bjacobi or asm |
| `-pc_hypre_type` | `boomeramg` | Algebraic multigrid |
| `-pc_hypre_boomeramg` (cont.) |   | "Information Threshold" for AMG process |
| `_strong_threshold` | `0.0 - 1.0` | **(0.7 is auto set for 3D) |
| `-ksp_gmres_restart` | `#` | Number of Krylov vectors to store |

!syntax parameters /Executioner/Steady

!syntax inputs /Executioner/Steady

!syntax children /Executioner/Steady
