# ADHeatFlux3EqnBC

!syntax description /BCs/ADHeatFlux3EqnBC

The heat flux contribution to the residual $R_i$ for the weak form of the energy equation is computed as:

\begin{equation}
R_i = (\psi_i, -S q_{wall} P_{hf}) \quad \forall \psi_i,
\end{equation}
where $\psi_i$ are the test functions and $q_{wall}$ is the local heat flux, $S$ a scaling factor
and $P_{hf}$ the heated perimeter.

!alert note
In THM, most boundary conditions are added automatically by components. This boundary condition is created by the
[HeatTransferFromHeatStructure1Phase.md] on the heat structure boundary.

!syntax parameters /BCs/ADHeatFlux3EqnBC

!syntax inputs /BCs/ADHeatFlux3EqnBC

!syntax children /BCs/ADHeatFlux3EqnBC
