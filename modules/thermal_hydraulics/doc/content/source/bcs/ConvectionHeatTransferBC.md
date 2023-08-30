# ConvectionHeatTransferBC

!syntax description /BCs/ConvectionHeatTransferBC

The heat flux contribution to the residual $R_i$ for the weak form of the energy equation is computed as:

\begin{equation}
R_i = (\psi_i, h_{ambient}(t, \vec{x}) (T - T_{ambient}(t, \vec{x}) ) ) \quad \forall \psi_i,
\end{equation}

where $\psi_i$ are the test functions, $h_{ambient}$ the ambient convection heat transfer coefficient, $T$ the temperature nonlinear
variable, $T_{ambient}$ the ambient temperature.

!alert note
In THM, most boundary conditions are added automatically by components. This boundary condition is no-longer in use, having
been replaced by its [AD](automatic_differentiation/index.md) counterpart [ADConvectionHeatTransferBC.md],
designed to provide numerically exact contributions to the Jacobian.

!syntax parameters /BCs/ConvectionHeatTransferBC

!syntax inputs /BCs/ConvectionHeatTransferBC

!syntax children /BCs/ConvectionHeatTransferBC
