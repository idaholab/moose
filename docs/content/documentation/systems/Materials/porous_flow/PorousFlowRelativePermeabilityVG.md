# PorousFlowRelativePermeabilityVG
!syntax description /Materials/PorousFlowRelativePermeabilityVG

The relative permeability of the phase is
\begin{equation}
k_{\mathrm{r}} = \sqrt{S_{\mathrm{eff}}} \left(1 - (1 -
S_{\mathrm{eff}}^{1/m})^{m} \right)^{2}, \nonumber
\end{equation}
where the effective saturation is
\begin{equation}
S_{\mathrm{eff}}(S) = \frac{S - S_{\mathrm{res}}^{\beta}}{1 -
  \sum_{\beta'}S_{\mathrm{res}}^{\beta'}}. \nonumber
\end{equation}

A *cut* version of the van Genuchten expression is also offered, which is
almost definitely indistinguishable experimentally from the original expression:
\begin{equation}
k_{\mathrm{r}} = \left\{
\begin{array}{ll}
\mbox{van Genuchten} & \mbox{ for } S_{\mathrm{eff}} < S_{c} \\
\mbox{cubic} & \mbox{ for } S_{\mathrm{eff}} \geq S_{c}. \nonumber
\end{array}
\right.
\end{equation}
Here the cubic is chosen so that its value and derivative match the
van Genuchten expression at $S=S_{c}$, and so that it is unity at
$S_{\mathrm{eff}}=1$.

!syntax parameters /Materials/PorousFlowRelativePermeabilityVG

!syntax inputs /Materials/PorousFlowRelativePermeabilityVG

!syntax children /Materials/PorousFlowRelativePermeabilityVG
