# ADWallHeatTransferCoefficientSchadMaterial

The material computes the convective heat transfer coefficient for liquid sodium in a rod bundle using the Schad-modified correlation.

Equation (10.130) from [!cite](todreas2021nuclear) is used.

The Nusselt number is calculated as:

\begin{equation}
  \text{Nu} = \begin{cases}
      \left[-16.15 + 24.96\left(\frac{\text{P}}{\text{D}}\right) - 8.55\left(\frac{\text{P}}{\text{D}}\right)^2 \right]\text{Pe}^{0.3} & \text{if $150 \leq \text{Pe} \leq 1000$}\\
      \\
      4.496 \left[-16.15 + 24.96\left(\frac{\text{P}}{\text{D}}\right) - 8.55\left(\frac{\text{P}}{\text{D}}\right)^2 \right] & \text{if $\text{Pe}<150$}\\
    \end{cases}
\end{equation}

with Pe given by

\begin{equation}
  \text{Pe} = \text{RePr} = \frac{{c_p}\rho D_h}{\mu}
\end{equation}

where P/D is the pitch-to-diameter ratio, $c_p$ is the heat capacity, $\rho$ is the density, $\mu$ is the viscosity, and $D_h$ is the hydraulic diameter.

The Schad correlation is valid for $1.1\leq \text{P/D} \leq 1.5$. The convective heat transfer coefficient, $h$, is calculated as:

\begin{equation}
  h = \frac{\text{Nu}k}{D_h}
\end{equation}

where $k$ is the fluid thermal conductivity.

!syntax parameters /Materials/ADWallHeatTransferCoefficientSchadMaterial

!syntax inputs /Materials/ADWallHeatTransferCoefficientSchadMaterial

!syntax children /Materials/ADWallHeatTransferCoefficientSchadMaterial
