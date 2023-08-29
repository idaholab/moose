# ADHSHeatFluxBC

!syntax description /BCs/ADHSHeatFluxBC

The heat flux contribution to the residual $R_i$ for the weak form of the energy equation is computed as:

\begin{equation}
R_i = (\psi_i, - S_{pp} S_{fn}(t, \vec{x}) F(t, \vec{x})) \quad \forall \psi_i,
\end{equation}

where $\psi_i$ are the test functions, $S_{pp}$ a postprocessor scaling factor, $S_{fn}$ a scaling
function, $F(t, \vec{x})$ a function providing the heat flux.

!alert note
In THM, most boundary conditions are added automatically by components. This boundary condition is created by the
[HSBoundaryHeatFlux.md] boundary heat structure.

!syntax parameters /BCs/ADHSHeatFluxBC

!syntax inputs /BCs/ADHSHeatFluxBC

!syntax children /BCs/ADHSHeatFluxBC
