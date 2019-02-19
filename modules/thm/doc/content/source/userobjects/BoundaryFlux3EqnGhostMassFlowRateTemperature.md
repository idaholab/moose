!syntax description /UserObjects/BoundaryFlux3EqnGhostMassFlowRateTemperature

!include euler_1d_var_area_boundary_flux_ghost.md

This computes the boundary flux from a specified mass flow rate $\dot{m}=\rho u A$
and temperature $T$, with pressure $p_i$ coming from the interior solution:
\begin{equation}
  \mathbf{U}_b = \mathbf{U}(\dot{m}, T, p_i) \,.
\end{equation}

!syntax parameters /UserObjects/BoundaryFlux3EqnGhostMassFlowRateTemperature

!syntax inputs /UserObjects/BoundaryFlux3EqnGhostMassFlowRateTemperature

!syntax children /UserObjects/BoundaryFlux3EqnGhostMassFlowRateTemperature
