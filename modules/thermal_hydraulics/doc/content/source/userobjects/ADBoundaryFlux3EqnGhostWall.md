# ADBoundaryFlux3EqnGhostWall

!syntax description /UserObjects/ADBoundaryFlux3EqnGhostWall

!include euler_1d_var_area_boundary_flux_ghost.md

For the wall boundary condition, the velocity in the ghost cell has the
opposite sign as the interior velocity, but the thermodynamic state is the
same:
\begin{equation}
  \mathbf{U}_b = \begin{bmatrix}
    \rho_i A_i\\
    -\rho_i u_i A_i\\
    \rho_i E_i A_i\\
  \end{bmatrix} \,.
\end{equation}

!syntax parameters /UserObjects/ADBoundaryFlux3EqnGhostWall

!syntax inputs /UserObjects/ADBoundaryFlux3EqnGhostWall

!syntax children /UserObjects/ADBoundaryFlux3EqnGhostWall
