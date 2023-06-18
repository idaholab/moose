# MomentumFluxIntegral

!syntax description /Postprocessors/MomentumFluxIntegral

The integral of the momentum flux $I_M$ over boundary $\partial \Omega$ is:

!equation
I_M = \int_{\partial \Omega} arhouA * velocity + \alpha * p * A d\partial \Omega

with $arhouA$ the conserved phase momentum, $velocity$ the boundary 1D velocity,
$\alpha$ the phase fraction, $p$ the pressure and $A$ the channel area at the boundary.

!syntax parameters /Postprocessors/MomentumFluxIntegral

!syntax inputs /Postprocessors/MomentumFluxIntegral

!syntax children /Postprocessors/MomentumFluxIntegral
