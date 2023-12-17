# FVPorousFlowHeatAdvection

!syntax description /FVKernels/FVPorousFlowHeatAdvection

This `FVKernel` implements the strong form of
\begin{equation*}
  -\nabla\cdot \sum_{\beta}h_{\beta} \rho_{\beta}\frac{k\,k_{\mathrm{r,}\beta}}{\mu_{\beta}}(\nabla P_{\beta} - \rho_{\beta} \mathbf{g})
\end{equation*}
where all parameters are defined in the [nomenclature](/nomenclature.md).

!syntax parameters /FVKernels/FVPorousFlowHeatAdvection

!syntax inputs /FVKernels/FVPorousFlowHeatAdvection

!syntax children /FVKernels/FVPorousFlowHeatAdvection
