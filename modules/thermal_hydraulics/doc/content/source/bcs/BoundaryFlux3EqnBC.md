# BoundaryFlux3EqnBC

!syntax description /BCs/BoundaryFlux3EqnBC

This implements a general boundary condition for the 1-D, 1-phase, variable-area Euler
equations, using a boundary flux user object, which computes the boundary flux
$F_b$ used in the boundary integral:
\begin{equation}
  \int\limits_\Gamma F_b(\mathbf{U}) \phi_i n_x d\Gamma \,.
\end{equation}

!alert note title=Linear Cross-sectional Area Used in Boundary Flux for RDG
For RDG spatial discretization, the solution in the boundary cells is not
reconstructed, so the cell-average solution is used. However, the cross-sectional
area used is the piecewise linear area $A_b$, not the cell-average area. In implementation,
only the cell-average area $A_i$ is passed into the boundary flux function, so the
linear cross-sectional area is used externally as follows:
\begin{equation}
  \int\limits_\Gamma F_b(\mathbf{U}) \frac{A_b}{A_i} \phi_i n_x d\Gamma \,.
\end{equation}

!syntax parameters /BCs/BoundaryFlux3EqnBC

!syntax inputs /BCs/BoundaryFlux3EqnBC

!syntax children /BCs/BoundaryFlux3EqnBC
