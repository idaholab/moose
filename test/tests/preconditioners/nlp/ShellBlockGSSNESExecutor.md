# ShellBlockGSSNESExecutor

!syntax description /Executors/ShellBlockGSSNESExecutor

## Overview

`ShellBlockGSSNESExecutor` implements nonlinear block Gauss-Seidel
preconditioning by sweeping over a set of sub-SNES executors. For the standard
forward multiplicative sweep, each block solve uses the latest solution from
the blocks that have already been swept and the incoming solution from blocks
that have not yet been swept.

PETSc's nonlinear preconditioning residual is the fixed-point residual

!equation id=shell-block-gs-npc-residual
P(x) = x - M(x),

where $M(x)$ is the result of applying one nonlinear preconditioning sweep to
the current nonlinear iterate $x$.

## Two-block multiplicative Jacobian

For a two-block residual

!equation id=shell-block-gs-original-residual
F(x) =
\begin{bmatrix}
F_0(u, v) \\
F_1(u, v)
\end{bmatrix},

the forward multiplicative sweep computes $M(u, v) = (\hat{u}, \hat{v})$ from

!equation id=shell-block-gs-forward-sweep
F_0(\hat{u}, v) = 0, \qquad F_1(\hat{u}, \hat{v}) = 0.

The preconditioned residual is therefore

!equation id=shell-block-gs-preconditioned-residual
P(u, v) =
\begin{bmatrix}
u - \hat{u} \\
v - \hat{v}
\end{bmatrix}.

At a fixed point, write the Jacobian of the original residual as

!equation id=shell-block-gs-original-jacobian
J =
\begin{bmatrix}
J_{00} & J_{01} \\
J_{10} & J_{11}
\end{bmatrix}.

Assuming exact block solves and nonsingular diagonal blocks, the Jacobian of the
preconditioned residual is

!equation id=shell-block-gs-preconditioned-jacobian
J_P =
\begin{bmatrix}
I & J_{00}^{-1} J_{01} \\
0 & I - J_{11}^{-1} J_{10} J_{00}^{-1} J_{01}
\end{bmatrix}.

Equivalently, this is the original Jacobian left-preconditioned by the inverse
of its lower block-triangular factor:

!equation id=shell-block-gs-lower-factor
J_P = L^{-1} J,
\qquad
L =
\begin{bmatrix}
J_{00} & 0 \\
J_{10} & J_{11}
\end{bmatrix}.

This is the analytic form implemented by the matrix-free linearization for the
simple multiplicative sweep: apply the full Jacobian, then apply one block
Gauss-Seidel solve with the lower block-triangular factor.

## Two-block symmetric multiplicative Jacobian

For the symmetric multiplicative sweep, the forward sweep is followed by a
backward sweep that excludes the last block solved by the forward sweep. For two
blocks this gives the sequence $0, 1, 0$.

At a fixed point, define the upper block-triangular factor and block diagonal
factor of $J$ as

!equation id=shell-block-gs-upper-and-diagonal-factors
U =
\begin{bmatrix}
J_{00} & J_{01} \\
0 & J_{11}
\end{bmatrix},
\qquad
D =
\begin{bmatrix}
J_{00} & 0 \\
0 & J_{11}
\end{bmatrix}.

Then the Jacobian of the symmetrically preconditioned residual is

!equation id=shell-block-gs-symmetric-preconditioned-jacobian
J_{P,\mathrm{sym}} = U^{-1} D L^{-1} J.

This is the block symmetric Gauss-Seidel form: apply the full Jacobian, apply
the lower block-triangular solve from the forward sweep, and then apply the
upper block-triangular correction from the backward sweep without counting the
diagonal block solves twice.

!syntax parameters /Executors/ShellBlockGSSNESExecutor

!syntax inputs /Executors/ShellBlockGSSNESExecutor

!syntax children /Executors/ShellBlockGSSNESExecutor
