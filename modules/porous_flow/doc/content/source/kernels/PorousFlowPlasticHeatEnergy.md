# PorousFlowPlasticHeatEnergy

!syntax description /Kernels/PorousFlowPlasticHeatEnergy

This `Kernel` implements the weak form of
\begin{equation*}
  -\nu (1-\phi)\sigma^{\mathrm{eff}}_{ij}\frac{\partial}{\partial t}\epsilon_{ij}^{\mathrm{plastic}}
\end{equation*}
where all parameters are defined in the [nomenclature](/nomenclature.md).  Hence, this Kernel models the heating produced by inelastic deformation of the porous skeleton.

Some examples may be found in [the test descriptions](porous_flow/tests/plastic_heating/plastic_heating_tests.md)

!syntax parameters /Kernels/PorousFlowPlasticHeatEnergy

!syntax inputs /Kernels/PorousFlowPlasticHeatEnergy

!syntax children /Kernels/PorousFlowPlasticHeatEnergy
