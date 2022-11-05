# ScalarLMKernel

This Kernel implements part of the equation that enforces the constraint of

\begin{equation}
 \int_{\Omega} \phi = V_0
\end{equation}

where $V_0$ is a given constant, using a Lagrange multiplier approach. The residual of $\phi$ related to the Lagrange multiplier is given as:

\begin{equation}
  F^{(\phi)}_i \equiv \lambda^h \int_{\Omega} \varphi_i \;\text{d}\Omega \label{eq:eq1}
\end{equation}

In particular, this Kernel implements the residual contribution from
the $\lambda$ in [eq:eq1] , and their Jacobian contributions. See also [AverageValueConstraint.md] for the residual of the Lagrange multiplier variable.

The detailed description of the derivation can be found at [scalar_constraint_kernel](https://github.com/idaholab/large_media/blob/master/framework/scalar_constraint_kernel.pdf).

!syntax parameters /Kernels/ScalarLMKernel

!syntax inputs /Kernels/ScalarLMKernel

!syntax children /Kernels/ScalarLMKernel
