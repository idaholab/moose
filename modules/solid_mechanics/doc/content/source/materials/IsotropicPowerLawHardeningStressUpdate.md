# Isotropic Power Law Hardening Stress Update

!syntax description /Materials/IsotropicPowerLawHardeningStressUpdate

## Description

!include modules/tensor_mechanics/common/supplementalRadialReturnStressUpdate.md

This class models power law hardening by using the relation
\begin{equation}
\sigma = \sigma_y + K \epsilon^n
\end{equation}
where $\sigma_y$ is the yield stress. This class solves for the yield stress as the intersection of
the power law relation curve and Hooke's law:
\begin{equation}
\epsilon_y = \frac{\sigma_y}{E} = \left( \frac{\sigma_y}{K} \right)^n
\end{equation}
where $\epsilon_y$ is the total strain at the yield point and the stress $\sigma_y$ is the von Mises
stress. Parameters from the parent class,
[IsotropicPlasticityStressUpdate](/IsotropicPlasticityStressUpdate.md), are suppressed to enable this
class to solve for yield stress:
\begin{equation}
\sigma_y = \left( \frac{E^n}{K} \right)^{1/(n-1)}
\end{equation}

## Example Input File Syntax

!listing modules/combined/test/tests/power_law_hardening/PowerLawHardening.i
         block=Materials/power_law_hardening

`IsotropicPowerLawHardeningStressUpdate` must be run in conjunction with the inelastic strain return
mapping stress calculator as shown below:

!listing modules/combined/test/tests/power_law_hardening/PowerLawHardening.i
         block=Materials/radial_return_stress

!syntax parameters /Materials/IsotropicPowerLawHardeningStressUpdate

!syntax inputs /Materials/IsotropicPowerLawHardeningStressUpdate

!syntax children /Materials/IsotropicPowerLawHardeningStressUpdate
