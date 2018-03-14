# PorousFlowRelativePermeabilityFLAC

!syntax description /Materials/PorousFlowRelativePermeabilityFLAC

The relative permeability of the phase is
\begin{equation*}
k_{\mathrm{r}} = (m + 1)S_{\mathrm{eff}}^{m} - m S_{\mathrm{eff}}^{m + 1},
\end{equation*}
where the effective saturation is
\begin{equation*}
S_{\mathrm{eff}}(S) = \frac{S - S_{\mathrm{res}}^{\beta}}{1 -
  \sum_{\beta'}S_{\mathrm{res}}^{\beta'}}.
\end{equation*}

!syntax parameters /Materials/PorousFlowRelativePermeabilityFLAC

!syntax inputs /Materials/PorousFlowRelativePermeabilityFLAC

!syntax children /Materials/PorousFlowRelativePermeabilityFLAC
