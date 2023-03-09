# FVPorousFlowAdvectiveFluxBC

!syntax description /FVBCs/FVPorousFlowAdvectiveFluxBC

This boundary condition implements the weak form of
\begin{equation*}
  -\nabla\cdot \sum_{\beta}\chi_{\beta}^{\kappa} \rho_{\beta}\frac{k\,k_{\mathrm{r,}\beta}}{\mu_{\beta}}(\nabla P_{\beta} - \rho_{\beta} \mathbf{g})
\end{equation*}
on the boundary where all parameters are defined in the [nomenclature](/nomenclature.md).

!syntax parameters /FVBCs/FVPorousFlowAdvectiveFluxBC

!syntax inputs /FVBCs/FVPorousFlowAdvectiveFluxBC

!syntax children /FVBCs/FVPorousFlowAdvectiveFluxBC
