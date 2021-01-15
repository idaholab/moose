# INSFVEnergyTimeDerivative

## Description

The `INSFVEnergyTimeDerivative` kernel implements a time derivative for the domain $\Omega$ given by

\begin{equation}
\underbrace{\rho c_p \frac{\partial u}{\partial t}}_{\textrm{INSFVEnergyTimeDerivative}} +
\sum_{i=1}^n \beta_i = 0 \in \Omega.
\end{equation}
where $\rho$ is the material density, $c_p$ is the specific heat, and the second term on the left hand side corresponds to the strong forms of
other kernels.

The Jacobian is computed with automatic differentiation.

!syntax parameters /FVKernels/INSFVEnergyTimeDerivative

!syntax inputs /FVKernels/INSFVEnergyTimeDerivative

!syntax children /FVKernels/INSFVEnergyTimeDerivative
