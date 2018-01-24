# PorousFlowAdvectiveFlux
!syntax description /Kernels/PorousFlowAdvectiveFlux

This `Kernel` implements the weak form of
\begin{equation*}
  -\nabla\cdot \sum_{\beta}\chi_{\beta}^{\kappa} \rho_{\beta}\frac{k\,k_{\mathrm{r,}\beta}}{\mu_{\beta}}(\nabla P_{\beta} - \rho_{\beta} \mathbf{g})
\end{equation*}
where all parameters are defined in the [nomenclature](/porous_flow/nomenclature.md).

!!! note
    A fully-upwinded version is implemented, where the mobility of the upstream nodes is used.

See [upwinding](/porous_flow/upwinding.md) for details.

!syntax parameters /Kernels/PorousFlowAdvectiveFlux

!syntax inputs /Kernels/PorousFlowAdvectiveFlux

!syntax children /Kernels/PorousFlowAdvectiveFlux
