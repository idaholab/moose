# MassFluxIntegral

!syntax description /Postprocessors/MassFluxIntegral

The integral of the mass flux $I_M$ over boundary $\partial \Omega$ is:

!equation
I_M = \int_{\partial \Omega} \alpha \rho u A d\partial \Omega

with $\alpha \rho u A$ the conserved phase momentum at the boundary.

!syntax parameters /Postprocessors/MassFluxIntegral

!syntax inputs /Postprocessors/MassFluxIntegral

!syntax children /Postprocessors/MassFluxIntegral
