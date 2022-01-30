!syntax description /UserObjects/BoundaryFlux3EqnGhostFreeInflow

!include euler_1d_var_area_boundary_flux_ghost.md

For the free inflow boundary condition, the user specifies the far-stream
density, velocity, and pressure, which completely defines the exterior state:
\begin{equation}
  \mathbf{U}_b = \mathbf{U}(\rho_\infty, u_\infty, p_\infty) \,.
\end{equation}

!syntax parameters /UserObjects/BoundaryFlux3EqnGhostFreeInflow

!syntax inputs /UserObjects/BoundaryFlux3EqnGhostFreeInflow

!syntax children /UserObjects/BoundaryFlux3EqnGhostFreeInflow
