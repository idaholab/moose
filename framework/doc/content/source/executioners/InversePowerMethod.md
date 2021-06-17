# InversePowerMethod

!syntax description /Executioner/InversePowerMethod

## Overview

Eigenvalue executioners such as this one intend on solving the eigenvalue problem described by:

!equation
Ax = \frac{1}{k}Bx,

where $A$ and $B$ are linear or nonlinear operators represented by kernels. To differentiate the $B$ kernels from the $A$ kernels, we must derive all $B$ kernels from `EigenKernel`. Currently we are only interested in the absolute minimum eigenvalue  $\frac{1}{k}$ and the corresponding eigenvector $x$ of the system. We are also not seeking the solutions of a general nonlinear eigenvalue problem, where the operators have nonlinear dependency on the eigenvalue.

## The inverse power method algorithm

1. Initialization

   !equation
   \begin{aligned}
   k^{(0)} &= k_0 \\
   x^{(0)} &= x_0
   \end{aligned}

1. Update x and k

   !equation
   \begin{aligned}
   x^{(n)} &= \frac{1}{k^{(n-1)}} A^{-1}Bx^{(n-1)} \\
   k^{(n)} &= k^{(n-1)} \frac{|Bx^{(n)}|}{|Bx^{(n-1)}|}
   \end{aligned}

1. Check the convergence

   !equation
   \frac{|x^{(n)}-x^{(n-1)}|}{|x^{(n)}|} < tol_x

   and

   !equation
   \frac{|k^{(n)}-k^{(n-1)}|}{|k^{(n)}|} < tol_k

   When either of them is not true, return Step 2, otherwise exit.


We notice immediately that $\frac{|Bx|}{k}$ remains constant during the iteration, so if we make $\frac{|Bx^{(0)}|}{k^{(0)}}$ equal to 1, the algorithm can be simplified a little:

1. Initialization

   !equation
   \begin{aligned}
   k^{(0)} &= k_0 \\
   x^{(0)} &= k_0 \frac{x_0}{|Bx_0|}
   \end{aligned}

1. Update x and k

   !equation
   \begin{aligned}
   x^{(n)} &= \frac{1}{k^{(n-1)}} A^{-1}Bx^{(n-1)} \\
   k^{(n)} &= |Bx^{(n)}|
   \end{aligned}

1. Check the convergence

   !equation
   \frac{|x^{(n)}-x^{(n-1)}|}{|x^{(n)}|} < tol_x

   and

   !equation
   \frac{|k^{(n)}-k^{(n-1)}|}{|k^{(n)}|} < tol_k

   When either of them is not true, return Step 2, otherwise exit.


Also in this simplified algorithm, the solution is automatically normalized making $|Bx|=k$. We can do postprocessing to normalize the solution so that $|x|=c$, where $|.|$ can be any norm and $c$ is a scalar constant.

If the minimum eigenvalue and the second smallest eigenvalue are close, i.e. the dominance ratio is about equal to one, the inverse power iteration converges very slowly. In such a case, we can apply accelerations, such as Chebyshev acceleration, based on the on-the-fly estimation of the dominance ratio.

The inverse power method is appealing because we can apply matrix-free schemes on evaluating $Ax - \frac{1}{k}Bx$. We can use PJFNK for inverting $A$ and we do not have to exactly assemble matrix $A$ for the preconditioning purpose.

!syntax parameters /Executioner/InversePowerMethod

!syntax inputs /Executioner/InversePowerMethod

!syntax children /Executioner/InversePowerMethod
