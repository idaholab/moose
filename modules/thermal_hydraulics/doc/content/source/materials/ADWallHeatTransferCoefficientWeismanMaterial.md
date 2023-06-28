# ADWallHeatTransferCoefficientWeismanMaterial

The material computes the convective heat transfer coefficient for water in a rod bundle using the Weisman correlation.

Equations (10.92b), (10.104a), and (10.104b)  from [!cite](todreas2021nuclear) are used.

The Nusselt number is calculated as:

\begin{equation}
  \text{Nu} = \begin{cases}
      0.023(\text{Re}^{0.8} \text{Pr}^n)\left[1.826 \left(\frac{\text{P}}{\text{D}}\right) - 1.0430 \right] & \text{for a square array rod bundle}\\
      \\
      0.023(\text{Re}^{0.8} \text{Pr}^n)\left[1.130 \left(\frac{\text{P}}{\text{D}}\right) - 0.2609 \right] & \text{for a triangular array rod bundle}\\
    \end{cases}
\end{equation}

with the Reynolds, Re, and Prandtl, Pr, numbers given by:

\begin{equation}
  \text{Re} = \frac{\rho v D_h}{\mu},\text{ and } \text{Pr} = \frac{c_p \mu}{k}
\end{equation}

where P/D is the pitch-to-diameter ratio, $c_p$ is the heat capacity, $\rho$ is the density, $\mu$ is the viscosity, $k$ is the fluid thermal conductivity, and $D_h$ is the hydraulic diameter. The constant $n$ is equal to $0.4$ for heating problems, and $0.3$ for cooling problems.

Weisman's correlation is valid for $0.7<\text{Pr}<100$ and $\text{Re}>10000$ for both bundle configurations. For a square array rod bundle, the Weisman correlation is valid for $1.1\leq \text{P/D} \leq 1.3$, while for a triangular array rod bundle it is valid for $1.1\leq \text{P/D} \leq 1.5$.


\begin{equation}
  h = \frac{\text{Nu}k}{D_h}
\end{equation}

!syntax parameters /Materials/ADWallHeatTransferCoefficientWeismanMaterial

!syntax inputs /Materials/ADWallHeatTransferCoefficientWeismanMaterial

!syntax children /Materials/ADWallHeatTransferCoefficientWeismanMaterial
