# WCNSFVScalarTimeDerivative

## Description

The `WCNSFVScalarTimeDerivative` kernel implements a time derivative for the domain $\Omega$ given by

\begin{equation}
\rho \frac{\partial u}{\partial t} +
\frac{\partial \rho}{\partial t} u +
\sum_{i=1}^n \beta_i = 0 \in \Omega.
\end{equation}
where $\rho$ is the material density, $u$ is the scalar being transported by the
fluid, and the last term on the left hand side corresponds to the strong forms of
other kernels.

The Jacobian is computed with automatic differentiation.

!syntax parameters /FVKernels/WCNSFVScalarTimeDerivative

!syntax inputs /FVKernels/WCNSFVScalarTimeDerivative

!syntax children /FVKernels/WCNSFVScalarTimeDerivative
