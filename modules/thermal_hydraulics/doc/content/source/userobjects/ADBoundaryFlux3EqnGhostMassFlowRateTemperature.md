# ADBoundaryFlux3EqnGhostMassFlowRateTemperature

!syntax description /UserObjects/ADBoundaryFlux3EqnGhostMassFlowRateTemperature

!include euler_1d_var_area_boundary_flux_ghost.md

This computes the boundary flux from a specified mass flow rate $\dot{m}=\rho u A$
and temperature $T$, with pressure $p_i$ coming from the interior solution:
\begin{equation}
  \mathbf{U}_b = \mathbf{U}(\dot{m}, T, p_i) \,.
\end{equation}

!syntax parameters /UserObjects/ADBoundaryFlux3EqnGhostMassFlowRateTemperature

!syntax inputs /UserObjects/ADBoundaryFlux3EqnGhostMassFlowRateTemperature

!syntax children /UserObjects/ADBoundaryFlux3EqnGhostMassFlowRateTemperature
