# ADWallHeatTransferCoefficientWolfMcCarthyMaterial

The material computes the convective heat transfer coefficient using the Wolf-McCarthy correlation.
Equation (25) from [!cite](wolf_mccarthy1960)) is used. Note that in the equation of the report, $T_{wall}$ and
$T_{fluid}$ are inverted. This is believed to be a typo as it is not consistent with the rest of the report.

The Nusselt number is calculated as:

\begin{equation}
  Nu= 0.025 Re^{0.8} Pr^{0.4} \left(\frac{T_{wall}}{T_{fluid}}\right)^{-0.55}
\end{equation}

!syntax parameters /Materials/ADWallHeatTransferCoefficientWolfMcCarthyMaterial

!syntax inputs /Materials/ADWallHeatTransferCoefficientWolfMcCarthyMaterial

!syntax children /Materials/ADWallHeatTransferCoefficientWolfMcCarthyMaterial
