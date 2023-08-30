# ExternalAppConvectionHeatTransferBC

!syntax description /BCs/ExternalAppConvectionHeatTransferBC

The contribution to the residual $R_i$ for the weak form of the energy equation is computed as:

\begin{equation}
R_i = (\psi_i, S_{pp} S_{fn}(t, \vec{x}) h_{ext} (T - T_{ext}) ) \quad \forall \psi_i,
\end{equation}

where $\psi_i$ are the test functions, $S_{pp}$ a postprocessor scaling factor, $S_{fn}$ a scaling
function, $h_{ext}$ is the heat transfer coefficient variable provided by the external application,
$T$ is the temperature variable on the heat structure, and $T_{ext}$ the temperature variable provided by the
external application

!alert note
In THM, most boundary conditions are added automatically by components. This boundary condition is no-longer in use, having
been replaced by its [AD](automatic_differentiation/index.md) counterpart [ADExternalAppConvectionHeatTransferBC.md],
designed to provide numerically exact contributions to the Jacobian.

!syntax parameters /BCs/ExternalAppConvectionHeatTransferBC

!syntax inputs /BCs/ExternalAppConvectionHeatTransferBC

!syntax children /BCs/ExternalAppConvectionHeatTransferBC
