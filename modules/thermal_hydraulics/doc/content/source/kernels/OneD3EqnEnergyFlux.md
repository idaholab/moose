# OneD3EqnEnergyFlux

!syntax description /Kernels/OneD3EqnEnergyFlux

This kernel implements an internal and kinetic energy advection term and a pressure work term after an integration by
parts.
The contribution to the residual $R_i$ for the weak form of the energy equation is computed as:

\begin{equation}
R_i = (\nabla \psi_i, -u \vec{d} (\rho * (e + \dfrac{u^2}{2}) + p) A) \quad \forall \psi_i,
\end{equation}
where $\nabla \psi_i$ is the gradient of each test function, $\rho$ is the density,
$e$ the specific internal energy, $u$ the one-dimensional velocity, $p$ the pressure,
$A$ the area of the component and $\vec{d}$ the direction of the flow channel.

!alert note
In THM, most kernels are added automatically by components. This kernel is no-longer in use.

!syntax parameters /Kernels/OneD3EqnEnergyFlux

!syntax inputs /Kernels/OneD3EqnEnergyFlux

!syntax children /Kernels/OneD3EqnEnergyFlux
