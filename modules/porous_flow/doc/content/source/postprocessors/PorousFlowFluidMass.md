# PorousFlowFluidMass

!syntax description /Postprocessors/PorousFlowFluidMass

This `Postprocessor` calculates the mass of a fluid component $\kappa$ using
\begin{equation*}
M^{\kappa} = \phi \sum_{\beta} \chi^{\kappa}_{\beta} \rho_{\beta} S_{\beta},
\end{equation*}
where all variables are defined in [`nomenclature`](/nomenclature.md).

The fluid component $\kappa$ is specified in the input parameter `fluid_component`.
By default, the mass of fluid component $\kappa$ is summed over all fluid phases. The
sum can be restricted to only a subset of fluid phases by entering the phase indexes
in the `phase` input parameter.

This `Postprocessor` also provides the option to only calculate fluid mass below a
certain saturation, which can be invoked using the `saturation_threshold` parameter.

!alert note
The flag `use_displaced_mesh = true` must be used in simulations experiencing solid-mechanical deformation.

!syntax parameters /Postprocessors/PorousFlowFluidMass

!syntax inputs /Postprocessors/PorousFlowFluidMass

!syntax children /Postprocessors/PorousFlowFluidMass
