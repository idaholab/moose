!syntax description /UserObjects/BoundaryFlux3EqnGhostFreeOutflow

!include euler_1d_var_area_boundary_flux_ghost.md

For the free outflow boundary condition, the exterior state is defined to be
exactly the same as the interior state, and the user does not supply any boundary information:
\begin{equation}
  \mathbf{U}_b = \mathbf{U}_i \,.
\end{equation}
Assuming that the numerical flux satisfies the consistency condition,
\begin{equation}
  \mathcal{F}(\mathbf{U}, \mathbf{U}) = \mathbf{F}(\mathbf{U}) \,,
\end{equation}
the choice of numerical flux should not make any difference here.

!syntax parameters /UserObjects/BoundaryFlux3EqnGhostFreeOutflow

!syntax inputs /UserObjects/BoundaryFlux3EqnGhostFreeOutflow

!syntax children /UserObjects/BoundaryFlux3EqnGhostFreeOutflow
