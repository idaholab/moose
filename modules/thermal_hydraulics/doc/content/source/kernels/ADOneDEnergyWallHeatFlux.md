# ADOneDEnergyWallHeatFlux

!syntax description /Kernels/ADOneDEnergyWallHeatFlux

The heat flux contribution to the residual $R_i$ for the weak form is computed as:

\begin{equation}
R_i = (\psi_i, -q_{wall} P_{hf}) \quad \forall \psi_i,
\end{equation}

where $\psi_i$ are the test functions, $q_{wall}$ is the local heat flux, and $P_{hf}$ is the
heated perimeter.

!alert note
In THM, most kernels are added automatically by components. This kernel is created by the
heat transfer components such as the [HeatTransferFromHeatFlux1Phase.md].

!syntax parameters /Kernels/ADOneDEnergyWallHeatFlux

!syntax inputs /Kernels/ADOneDEnergyWallHeatFlux

!syntax children /Kernels/ADOneDEnergyWallHeatFlux
