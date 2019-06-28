# PorousFlowRelativePermeabilityBC

!syntax description /Materials/PorousFlowRelativePermeabilityBC

The [!cite](brookscorey1966) relative permeability model is an extension of the previous  [!cite](corey1954) formulation where the relative permeability of the wetting phase is given by
\begin{equation*}
k_{\mathrm{r, w}} = \left(S_{\mathrm{eff}}\right)^{(2 + 3 \lambda)/\lambda},
\end{equation*}
and the relative permeability of the non-wetting phase is
\begin{equation*}
k_{\mathrm{r, nw}} = (1 - S_{\mathrm{eff}})^2 \left[1 - \left(S_{\mathrm{eff}}\right)^{(2 + \lambda)/\lambda}\right],
\end{equation*}
where the effective saturation is
\begin{equation*}
S_{\mathrm{eff}}(S) = \frac{S - S_{\mathrm{res}}^{\beta}}{1 -
  \sum_{\beta'}S_{\mathrm{res}}^{\beta'}},
\end{equation*}
and $\lambda$ is a user-defined exponent.

!syntax parameters /Materials/PorousFlowRelativePermeabilityBC

!syntax inputs /Materials/PorousFlowRelativePermeabilityBC

!syntax children /Materials/PorousFlowRelativePermeabilityBC


!bibtex bibliography

