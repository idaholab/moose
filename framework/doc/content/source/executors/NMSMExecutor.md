# NMSMExecutor

!syntax description /Executors/NMSMExecutor

## Overview

`NMSMExecutor` implements nonlinear block Gauss-Seidel
by sweeping over a set of sub-SNES executors. For the standard
forward multiplicative sweep, each block solve uses the latest solution from
the blocks that have already been swept and the incoming solution from blocks
that have not yet been swept. If this class is used as a preconditioner for
[NewtonSNESExecutor.md] then the combination is known as MSPIN for Multiplicative
Schwarz Preconditioned Inexact Newton [!citep](liu2015field). It is also nonlinear
field split preconditioning; the nonlinear analog of [FieldSplitPreconditioner.md].
`NMSMExecutor` may also be used as a stand-alone nonlinear solver; however, just as
linear Gauss-Seidel is itself a poor linear solver, its much more appropriately
used as a preconditioner/smoother. We encourage the reader to read the Liu and Keyes
reference as it gives the definition of the left nonlinearly preconditioned residual
as well as how to compute its exact Jacobian. For clarity we reproduce the MSPIN
Jacobian here for a two-field $x = [u,v]^T$ split:

!equation id=preconditioned-Jacobian
\mathcal{J}(u, v) =
\begin{bmatrix}
\frac{\partial G}{\partial p} & 0 \\
\frac{\partial H}{\partial p} & \frac{\partial H}{\partial q}
\end{bmatrix}^{-1}
\begin{bmatrix}
\frac{\partial G}{\partial p} & \frac{\partial G}{\partial v} \\
\frac{\partial H}{\partial p} & \frac{\partial H}{\partial q}
\end{bmatrix}

where $G$ corresponds to the vector-valued residual function for the $u$ field,
$H$ corresponds to the vector-vallued residual function for the $v$ field, $p$
corresponds to the $u$ field after nonlinear preconditioner corrections, and
$q$ corresponds to the $v$ field after nonlinear preconditioner corrections.
$p$ is equivalent to $u - g$ and $q$ is equivalent to $v - h$, such that $g$ and
$h$ are the nonlinear preconditioner corrections to $u$ and $v$. The preconditioned
residual is equated to these corrections

!equation id=preconditioned-residual
\mathcal{F}(u, v) =
\begin{bmatrix}
g(u, v) \\
h(u, v)
\end{bmatrix}

such that Newton's method continues until the corrections approach zero. Looking at
[preconditioned-Jacobian], it's clear that the exact Jacobian for the preconditioned
residual involves taking derivative evaluations of the residual functions $G$ and
$H$ at different solution points, most often with the corrected solutions $p$ and $q$,
but sometimes with the uncorrected solution $v$. In Keyes's paper they state that
in their implementation it easiest to evaluate the unpreconditioned Jacobian function
with the $[p, v]^T$ solution pair. In our MSPIN implementation we choose to use the
$[p, q]^T$ solution pair, which is consistent with PETSc's default for the additive
Scharz variant ASPIN. We note that as Newton approaches converge $p\rightarrow u$ and
$q\rightarrow v$ such that the approximation vanishes.

We note that the MSPIN, forward Gauss-Seidel Jacobian can be written compactly as

!equation id=compact-mspin
\mathcal{J}_{GS} = L^{-1}J

where $L$ contains the lower-triangular components of $J$. If [!param](/Executors/NMSMExecutor/sweep_type) is
set to `symmetric_multiplicative` then the nonlinearly symmetric Gauss-Seidel (SGS) preconditioned Jacobian becomes

!equation id=compact-symm-mult-pin
\mathcal{J}_{SGS} = U^{-1} D L^{-1} J

where $U$ includes the upper-triangular components of $J$ and $D$ includes the diagonal
components of $J$.\ In words, we apply the full Jacobian, apply
the lower block-triangular solve from the forward Gauss-Seidel sweep, and then apply the
upper block-triangular correction from the backward sweep without double-counting
diagonal block solves (hence the appearance of $D$).

!syntax parameters /Executors/NMSMExecutor

!syntax inputs /Executors/NMSMExecutor

!syntax children /Executors/NMSMExecutor
