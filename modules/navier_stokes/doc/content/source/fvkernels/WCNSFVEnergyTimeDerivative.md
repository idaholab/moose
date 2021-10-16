# WCNSFVEnergyTimeDerivative

## Description

The `WCNSFVEnergyTimeDerivative` kernel implements a time derivative for the domain $\Omega$ given by

\begin{equation}
\rho c_p \frac{\partial u}{\partial t} +
\rho \frac{\partial c_p}{\partial t} u +
\frac{\partial \rho}{\partial t} c_p u +
\sum_{i=1}^n \beta_i = 0 \in \Omega.
\end{equation}
where $\rho$ is the material density, $c_p$ is the specific heat, and the second term on the left hand side corresponds to the strong forms of
other kernels.

The Jacobian is computed with automatic differentiation.

!syntax parameters /FVKernels/WCNSFVEnergyTimeDerivative

!syntax inputs /FVKernels/WCNSFVEnergyTimeDerivative

!syntax children /FVKernels/WCNSFVEnergyTimeDerivative
