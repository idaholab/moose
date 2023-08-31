# ADBoundaryFlux3EqnBC

!syntax description /BCs/ADBoundaryFlux3EqnBC

The boundary conditions retrieves the flux from an `ADBoundaryFluxBase`-derived user object,
such as the [ADBoundaryFlux3EqnFreeOutflow.md].

The heat flux contribution to the residual $R_i$ for the weak form of the energy equation is computed as:

\begin{equation}
R_i = (\psi_i, \Phi_{eqn} * \vec{n}) \quad \forall \psi_i,
\end{equation}

where $\psi_i$ are the test functions, $\Phi_{eqn}$ the flux for the equation considered, and $\vec{n}$ the local normal.

!alert note
In THM, most boundary conditions are added automatically by components. This boundary condition is created by
`FlowBoundary1Phase`-derived components, such as the [InletDensityVelocity1Phase.md] boundary component.

!syntax parameters /BCs/ADBoundaryFlux3EqnBC

!syntax inputs /BCs/ADBoundaryFlux3EqnBC

!syntax children /BCs/ADBoundaryFlux3EqnBC
