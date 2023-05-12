# ADWallHeatTransferCoefficientKazimiMaterial

The material computes the convective heat transfer coefficient for liquid sodium in a rod bundle using the Kazimi and Carelli correlation. Equation (10.129) from [!cite](todreas2021nuclear) is used.

The Nusselt number is calculated as:

\begin{equation}
  \text{Nu} = 4.0 + 0.33\left(\frac{\text{P}}{\text{D}}\right)^{3.8}\left(\frac{\text{Pe}}{100}\right)^{0.86} + 0.16\left(\frac{\text{P}}{\text{D}}\right)^{5.0}
\end{equation}

with Pe given by

\begin{equation}
  \text{Pe} = \text{RePr} = \frac{{c_p}\rho D_h}{\mu}
\end{equation}

where P/D is the pitch-to-diameter ratio, $c_p$ is the heat capacity, $\rho$ is the density, $\mu$ is the viscosity, and $D_h$ is the hydraulic diameter.

The Kazimi and Carelli correlation is valid for $1.1\leq \text{P/D} \leq 1.4$ and $10\leq \text{Pe} \leq 5000$. The convective heat transfer coefficient, $h$, is calculated as:

\begin{equation}
  h = \frac{\text{Nu}k}{D_h}
\end{equation}

where $k$ is the fluid thermal conductivity.

!syntax parameters /Materials/ADWallHeatTransferCoefficientKazimiMaterial

!syntax inputs /Materials/ADWallHeatTransferCoefficientKazimiMaterial

!syntax children /Materials/ADWallHeatTransferCoefficientKazimiMaterial
