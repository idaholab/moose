# AverageValueConstraint

This Kernel implements part of the equation that enforces the constraint of

\begin{equation}
 \int_{\Omega} \phi = V_0
\end{equation}

where $V_0$ is a given constant, using a Lagrange multiplier approach. The residual of the Lagrange multiplier is given as:

\begin{equation}
  F^{(\lambda)} \equiv \int_{\Omega} \phi^h \;\text{d}\Omega - V_0 = 0 \label{eq:eq1}
\end{equation}

In particular, this Kernel implements the residual contribution for
the $\lambda$ in [eq:eq1]. See also [ScalarLagrangeMultiplier](source/kernels/ScalarLagrangeMultiplier.md) for the residual of the variable $\phi$.

The detailed description of the derivation can be found at [scalar_constraint_kernel](https://github.com/idaholab/large_media/blob/master/framework/scalar_constraint_kernel.pdf).

!syntax parameters /ScalarKernels/AverageValueConstraint

!syntax inputs /ScalarKernels/AverageValueConstraint

!syntax children /ScalarKernels/AverageValueConstraint
