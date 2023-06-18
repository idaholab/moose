# EnergyFluxIntegral

!syntax description /Postprocessors/EnergyFluxIntegral

The integral of the energy flux $I_E$ over boundary $\partial \Omega$ is:

!equation
I_E = \int_{\partial \Omega} arhouA * h d\partial \Omega

with $arhouA$ the conserved phase momentum and $h$ the specific total enthalpy of the fluid at the boundary.

!syntax parameters /Postprocessors/EnergyFluxIntegral

!syntax inputs /Postprocessors/EnergyFluxIntegral

!syntax children /Postprocessors/EnergyFluxIntegral
