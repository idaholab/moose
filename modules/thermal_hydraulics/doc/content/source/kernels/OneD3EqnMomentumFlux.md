# OneD3EqnMomentumFlux

!syntax description /Kernels/OneD3EqnMomentumFlux

This kernel implements a momentum advection and pressure gradient flux after an integration by
parts.
The contribution to the residual $R_i$ for the weak form of the momentum equation is computed as:

\begin{equation}
R_i = (\nabla \psi_i, -(\rho u^2 + p) A \vec{d}) \quad \forall \psi_i,
\end{equation}
where $\nabla \psi_i$ is the gradient of each test function, $\rho$ is the density,
$A$ the area of the component, $u$ the one-dimensional velocity, $p$ the pressure, and
$\vec{d}$ the direction of the flow channel.

!syntax parameters /Kernels/OneD3EqnMomentumFlux

!syntax inputs /Kernels/OneD3EqnMomentumFlux

!syntax children /Kernels/OneD3EqnMomentumFlux
