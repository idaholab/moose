!syntax description /UserObjects/BoundaryFlux3EqnGhostVelocityTemperature

!include euler_1d_var_area_boundary_flux_ghost.md

This computes the boundary flux from a specified velocity $vel$
and temperature $T$, with pressure $p_i$ coming from the interior solution:
\begin{equation}
  \mathbf{U}_b = \mathbf{U}(vel, T, p_i) \,.
\end{equation}

!syntax parameters /UserObjects/BoundaryFlux3EqnGhostVelocityTemperature

!syntax inputs /UserObjects/BoundaryFlux3EqnGhostVelocityTemperature

!syntax children /UserObjects/BoundaryFlux3EqnGhostVelocityTemperature
