!syntax description /UserObjects/BoundaryFlux3EqnFreeInflow

This class implements a boundary flux corresponding to the free inflow boundary
condition, in which the user supplies far-stream values for density, velocity,
and pressure:
\begin{equation}
  \mathbf{U}_b = \mathbf{U}(\rho_\infty, u_\infty, p_\infty) \,.
\end{equation}
The boundary flux is then computed as
\begin{equation}
  \mathbf{F}_b = \mathbf{F}(\mathbf{U}_b) \,.
\end{equation}

!syntax parameters /UserObjects/BoundaryFlux3EqnFreeInflow

!syntax inputs /UserObjects/BoundaryFlux3EqnFreeInflow

!syntax children /UserObjects/BoundaryFlux3EqnFreeInflow
