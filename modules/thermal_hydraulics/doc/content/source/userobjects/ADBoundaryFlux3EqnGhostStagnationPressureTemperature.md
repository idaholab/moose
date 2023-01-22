# ADBoundaryFlux3EqnGhostStagnationPressureTemperature

!syntax description /UserObjects/ADBoundaryFlux3EqnGhostStagnationPressureTemperature

!include euler_1d_var_area_boundary_flux_ghost.md

This computes the boundary flux from a specified stagnation pressure $p_{0,b}$
and stagnation temperature $T_{0,b}$, with velocity $u_i$ coming from the
solution:
\begin{equation}
  \mathbf{U}_b = \mathbf{U}(p_{0,b}, T_{0,b}, u_i) \,,
\end{equation}
\begin{equation}
  \mathbf{F}_b = \mathbf{F}(\mathbf{U}_b) \,.
\end{equation}

!syntax parameters /UserObjects/ADBoundaryFlux3EqnGhostStagnationPressureTemperature

!syntax inputs /UserObjects/ADBoundaryFlux3EqnGhostStagnationPressureTemperature

!syntax children /UserObjects/ADBoundaryFlux3EqnGhostStagnationPressureTemperature
