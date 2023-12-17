# FVPorousFlowEnergyTimeDerivative

!syntax description /FVKernels/FVPorousFlowEnergyTimeDerivative

This `FVKernel` implements the strong form of
\begin{equation*}
  \frac{\partial}{\partial t}\left((1-\phi)\rho_{R}C_{R}T + \phi\sum_{\beta}S_{\beta}\rho_{\beta}\mathcal{E}_{\beta}\right)
\end{equation*}
where all parameters are defined in the [nomenclature](/nomenclature.md).

!alert note
Presently, a first-order accurate implicit Euler time derivative is hard-coded.

!syntax parameters /FVKernels/FVPorousFlowEnergyTimeDerivative

!syntax inputs /FVKernels/FVPorousFlowEnergyTimeDerivative

!syntax children /FVKernels/FVPorousFlowEnergyTimeDerivative
