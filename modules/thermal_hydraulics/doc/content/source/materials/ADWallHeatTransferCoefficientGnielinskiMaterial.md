# ADWallHeatTransferCoefficientGnielinskiMaterial

The material computes the convective heat transfer coefficient using the Gnielinski correlation. Equations (9.93) and (10.96) from [!cite](todreas2021nuclear) are used.

The Nusselt number is calculated as:

\begin{equation}
  \text{Nu} = \frac{(\text{f}/8)(\text{Re}-1000)\text{Pr}}{1+12.7\sqrt{(\text{f}/8)}\left(\text{Pr}^{2/3}-1\right)}
\end{equation}

with friction factor, f, given by:

\begin{equation}
  \text{f} = \frac{1}{\left(1.82\text{log}_{10}(\text{Re}) - 1.64\right)^2}
\end{equation}

The Reynolds, Re, and Prandtl, Pr, numbers are:

\begin{equation}
  \text{Re} = \frac{\rho v D_h}{\mu},\text{ and } \text{Pr} = \frac{c_p \mu}{k}
\end{equation}

where $c_p$ is the heat capacity, $\rho$ is the density, $\mu$ is the viscosity, $k$ is the fluid thermal conductivity, $v$ is the flow velocity, and $D_h$ is the hydraulic diameter. Gnielinski's correlation is valid for $2300\leq\text{Re}\leq 5 \times 10^{6}$, and for $0.5<\text{Pr}<200$ for 6% accuracy, or $200<\text{Pr}<2000$ for 10% accuracy.

The heat transfer coefficient, h, will be given by:

\begin{equation}
  h = \frac{\text{Nu}k}{D_h}
\end{equation}

!syntax parameters /Materials/ADWallHeatTransferCoefficientGnielinskiMaterial

!syntax inputs /Materials/ADWallHeatTransferCoefficientGnielinskiMaterial

!syntax children /Materials/ADWallHeatTransferCoefficientGnielinskiMaterial
