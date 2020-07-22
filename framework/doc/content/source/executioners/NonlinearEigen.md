# NonlinearEigen

!syntax description /Executioner/NonlinearEigen

## Overview

Eigenvalue executioners such as this one intend on solving the eigenvalue problem described by:

!equation
Ax = \frac{1}{k}Bx,

where $A$ and $B$ are linear or nonlinear operators represented by kernels. To differentiate the $B$ kernels from the $A$ kernels, we must derive all $B$ kernels from `EigenKernel`. Currently we are only interested in the absolute minimum eigenvalue  $\frac{1}{k}$ and the corresponding eigenvector $x$ of the system. We are also not seeking the solutions of a general nonlinear eigenvalue problem, where the operators have nonlinear dependency on the eigenvalue.

## The nonlinear Newton method

From the above section, we can see the eigenvalue problem can be viewed as a nonlinear problem

!equation
\begin{aligned}
&Ax = \frac{1}{k}Bx, \\
&k = |Bx|,
\end{aligned}

so we can use the Newton method to solve it. However, to make the solving converge, we need to have a fairly close initial guess to the fundamental mode. This can be achieved with several free power iterations before the Newton iteration. We do not have to have $k$ as part of the solution vector. Instead we can apply the elimination technique and view the equation as

!equation
Ax = \frac{Bx}{|Bx|}.

Again we can use PJFNK (preconditioned Jacobian-free Newton Krylov) method to solve this nonlinear problem. The preconditioning matrix can affect the linear convergence in each Newton iteration. If there is a convergence issue, it is suggested to use $A-\frac{B}{|Bx|}$ and gradually reduce its complexity as the preconditioning matrix.

!syntax parameters /Executioner/NonlinearEigen

!syntax inputs /Executioner/NonlinearEigen

!syntax children /Executioner/NonlinearEigen
