# PorousFlowPlasticHeatEnergy
!syntax description /Kernels/PorousFlowPlasticHeatEnergy

This `Kernel` implements the weak form of
\begin{equation*}
  -\nu (1-\phi)\sigma^{\mathrm{eff}}_{ij}\frac{\partial}{\partial t}\epsilon_{ij}^{\mathrm{plastic}}
\end{equation*}
where all parameters are defined in the [nomenclature](/porous_flow/nomenclature.md).

!syntax parameters /Kernels/PorousFlowPlasticHeatEnergy

!syntax inputs /Kernels/PorousFlowPlasticHeatEnergy

!syntax children /Kernels/PorousFlowPlasticHeatEnergy
