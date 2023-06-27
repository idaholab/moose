# ADWallHeatTransferCoefficientMikityukMaterial

The material computes the convective heat transfer coefficient for liquid sodium in a rod bundle using the Mikityuk correlation.

Equation (10.133) from [!cite](todreas2021nuclear) is used.

The Nusselt number is calculated as:

\begin{equation}
  \text{Nu} = 0.047 \left[ 1 - exp\left(\frac{\text{P}}{\text{D}} -1 \right)\right](\text{Pe}^{0.77} + 250)
\end{equation}

with Pe given by

\begin{equation}
  \text{Pe} = \text{RePr} = \frac{{c_p}\rho D_h}{\mu}
\end{equation}

where P/D is the pitch-to-diameter ratio, $c_p$ is the heat capacity, $\rho$ is the density, $\mu$ is the viscosity, and $D_h$ is the hydraulic diameter.

The Mikityuk correlation is valid for $1.1\leq \text{P/D} \leq 1.5$ and $30\leq \text{Pe} \leq 5000$. The convective heat transfer coefficient, $h$, is calculated as:

\begin{equation}
  h = \frac{\text{Nu}k}{D_h}
\end{equation}

where $k$ is the fluid thermal conductivity.

!syntax parameters /Materials/ADWallHeatTransferCoefficientMikityukMaterial

!syntax inputs /Materials/ADWallHeatTransferCoefficientMikityukMaterial

!syntax children /Materials/ADWallHeatTransferCoefficientMikityukMaterial
