# OneDEnergyWallHeatFlux

!syntax description /Kernels/OneDEnergyWallHeatFlux

The heat flux contribution to the residual $R_i$ for the weak form is computed as:

\begin{equation}
R_i = (\psi_i, -q_{wall} P_{hf}) \quad \forall \psi_i,
\end{equation}
where $\psi_i$ are the test functions and $q_{wall}$ is the local heat flux, and $P_{hf}$ the
heated perimeter.

!alert note
The dependence of the heat flux on any non-linear variable is not considered by this kernel, as such, there
is no contribution to the Jacobian.

!alert note
In THM, most kernels are added automatically by components. This kernel is no-longer in use, having
been replaced by its [AD](automatic_differentiation/index.md) counterpart [ADOneD3EqnEnergyHeatFlux.md],
designed to provide numerically exact contributions to the Jacobian.

!syntax parameters /Kernels/OneDEnergyWallHeatFlux

!syntax inputs /Kernels/OneDEnergyWallHeatFlux

!syntax children /Kernels/OneDEnergyWallHeatFlux
