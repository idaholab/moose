# NodalEnergyFluxPostprocessor

!syntax description /Postprocessors/NodalEnergyFluxPostprocessor

The energy flux $\Phi$ through nodes is computed as:

!equation
\Phi = \sum_{nodes} \alpha \rho u A H

where $\alpha$ is the phase fraction, $\rho$ is the phase density, $u$ is the phase velocity,
$A$ the local area, and $H$ the specific total enthalpy.

!syntax parameters /Postprocessors/NodalEnergyFluxPostprocessor

!syntax inputs /Postprocessors/NodalEnergyFluxPostprocessor

!syntax children /Postprocessors/NodalEnergyFluxPostprocessor
