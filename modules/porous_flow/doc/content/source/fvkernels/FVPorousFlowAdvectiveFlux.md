# FVPorousFlowAdvectiveFlux

!syntax description /FVKernels/FVPorousFlowAdvectiveFlux

This `FVKernel` implements the weak form of
\begin{equation*}
  -\nabla\cdot \sum_{\beta}\chi_{\beta}^{\kappa} \rho_{\beta}\frac{k\,k_{\mathrm{r,}\beta}}{\mu_{\beta}}(\nabla P_{\beta} - \rho_{\beta} \mathbf{g})
\end{equation*}
where all parameters are defined in the [nomenclature](/nomenclature.md).

!syntax parameters /FVKernels/FVPorousFlowAdvectiveFlux

!syntax inputs /FVKernels/FVPorousFlowAdvectiveFlux

!syntax children /FVKernels/FVPorousFlowAdvectiveFlux
