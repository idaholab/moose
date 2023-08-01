# ADOneD3EqnEnergyHeatFlux

!syntax description /Kernels/ADOneD3EqnEnergyHeatFlux

\begin{equation}
R_i = (\psi_i, -q_{wall} P_{hf}) \quad \forall \psi_i,
\end{equation}
where $\psi_i$ are the test functions and $q_{wall}$ is the local heat flux, and $P_{hf}$ the
heated perimeter.

!alert note
In THM, most kernels are added automatically by components. This kernel is created by the
[HeatTransferFromHeatStructure1Phase.md] component to model a surface heat flux from the heat structure.

!syntax parameters /Kernels/ADOneD3EqnEnergyHeatFlux

!syntax inputs /Kernels/ADOneD3EqnEnergyHeatFlux

!syntax children /Kernels/ADOneD3EqnEnergyHeatFlux
