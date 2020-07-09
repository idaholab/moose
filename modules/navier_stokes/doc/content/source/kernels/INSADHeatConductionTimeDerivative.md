# INSADHeatConductionTimeDerivative

## Description

The `INSADHeatConductionTimeDerivative` kernel implements a time derivative for the domain $\Omega$ given by

\begin{equation}
\underbrace{\rho c_p \frac{\partial u}{\partial t}}_{\textrm{INSADHeatConductionTimeDerivative}} +
\sum_{i=1}^n \beta_i = 0 \in \Omega.
\end{equation}
where $\rho$ is the material density, $c_p$ is the specific heat, and the second term on the left hand side corresponds to the strong forms of
other kernels. The corresponding `INSADHeatConductionTimeDerivative` weak form using inner-product notation is

\begin{equation}
R_i(u_h) = (\psi_i, \rho c_p\frac{\partial u_h}{\partial t}) \quad \forall \psi_i,
\end{equation}
where $u_h$ is the approximate solution and $\psi_i$ is a finite element test function.

The Jacobian is given by automatic differentiation, and should be perfect as long as $c_p$ and $\rho$
are provided using `ADMaterial` derived objects.

!syntax parameters /Kernels/INSADHeatConductionTimeDerivative

!syntax inputs /Kernels/INSADHeatConductionTimeDerivative

!syntax children /Kernels/INSADHeatConductionTimeDerivative
