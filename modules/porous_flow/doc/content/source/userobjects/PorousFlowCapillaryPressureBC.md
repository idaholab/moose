# PorousFlowCapillaryPressureBC

!syntax description /UserObjects/PorousFlowCapillaryPressureBC

The Brooks-Corey capillary-pressure relationship is [!citep](brookscorey1966)

\begin{equation}
S_{\mathrm{eff}} = \left( \frac{P_c}{P_e} \right)^{-\lambda},
\end{equation}
or
\begin{equation}
P_c = P_e S_{\mathrm{eff}}^{-1/\lambda},
\end{equation}
where $P_e$, the threshold entry pressure, and $\lambda$, the exponent, are required
parameters.

!syntax parameters /UserObjects/PorousFlowCapillaryPressureBC

!syntax inputs /UserObjects/PorousFlowCapillaryPressureBC

!syntax children /UserObjects/PorousFlowCapillaryPressureBC


!bibtex bibliography

