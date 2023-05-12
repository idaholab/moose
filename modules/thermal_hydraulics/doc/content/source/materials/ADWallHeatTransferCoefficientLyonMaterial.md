# ADWallHeatTransferCoefficientLyonMaterial

The material computes the convective heat transfer coefficient for liquid sodium in a circular tube
with constant heat flux along and around the tube using the Lyon correlation. Equation (10.126a) from [!cite](todreas2021nuclear) is used.

The Nusselt number is calculated as:

\begin{equation}
  \text{Nu} = 7 + 0.025\text{Pe}^{0.8}
\end{equation}

with Pe given by

\begin{equation}
  \text{Pe} = \text{RePr} = \frac{{c_p}\rho D_h}{\mu}
\end{equation}

where $c_p$ is the heat capacity, $\rho$ is the density, $\mu$ is the viscosity, and $D_h$ is the hydraulic diameter. The convective heat transfer coefficient, $h$, is calculated as:

\begin{equation}
  h = \frac{\text{Nu}k}{D_h}
\end{equation}

where $k$ is the fluid thermal conductivity.

!syntax parameters /Materials/ADWallHeatTransferCoefficientLyonMaterial

!syntax inputs /Materials/ADWallHeatTransferCoefficientLyonMaterial

!syntax children /Materials/ADWallHeatTransferCoefficientLyonMaterial
