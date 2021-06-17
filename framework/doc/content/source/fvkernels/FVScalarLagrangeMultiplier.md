# FVScalarLagrangeMultiplier

This object implements the residuals that enforce the constraint

\begin{equation}
 \int_{\Omega} \phi = \int \phi_0
\end{equation}

using a Lagrange multiplier approach. E.g. this object enforces the constraint
that the average value of $\phi$ match $\phi_0$.

The detailed description of the derivation for the corresponding finite element
constraint can be found at
[scalar_constraint_kernel](https://github.com/idaholab/large_media/blob/master/framework/scalar_constraint_kernel.pdf). The
finite volume version can be obtained by simply substituting $1$ for
$\varphi$. Note that $\int \phi_0 = V_0$.

!syntax parameters /FVKernels/FVScalarLagrangeMultiplier

!syntax inputs /FVKernels/FVScalarLagrangeMultiplier

!syntax children /FVKernels/FVScalarLagrangeMultiplier
