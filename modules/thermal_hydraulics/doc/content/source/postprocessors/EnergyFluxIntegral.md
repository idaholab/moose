# EnergyFluxIntegral

!syntax description /Postprocessors/EnergyFluxIntegral

The integral of the energy flux $I_E$ over boundary $\partial \Omega$ is:

!equation
I_E = \int_{\partial \Omega} \alpha \rho u A h d\partial \Omega

with $\alpha \rho u A$ the conserved phase momentum and $h$ the specific total enthalpy of the fluid at the boundary.

!syntax parameters /Postprocessors/EnergyFluxIntegral

!syntax inputs /Postprocessors/EnergyFluxIntegral

!syntax children /Postprocessors/EnergyFluxIntegral
