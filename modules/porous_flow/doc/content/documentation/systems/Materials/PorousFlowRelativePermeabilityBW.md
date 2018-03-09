# PorousFlowRelativePermeabilityBW

!syntax description /Materials/PorousFlowRelativePermeabilityBW

The relative permeability of the phase is
\begin{equation*}
k_{\mathrm{r}} = K_{n} + \frac{K_{s} - K_{n}}{(c - 1)(c -
  S_{\mathrm{eff}})}S_{\mathrm{eff}}^{2},
\end{equation*}
where the effective saturation is
\begin{equation*}
S_{\mathrm{eff}}(S) = \frac{S - S_{\mathrm{res}}^{\beta}}{1 -
  \sum_{\beta'}S_{\mathrm{res}}^{\beta'}}.
\end{equation*}

!syntax parameters /Materials/PorousFlowRelativePermeabilityBW

!syntax inputs /Materials/PorousFlowRelativePermeabilityBW

!syntax children /Materials/PorousFlowRelativePermeabilityBW
