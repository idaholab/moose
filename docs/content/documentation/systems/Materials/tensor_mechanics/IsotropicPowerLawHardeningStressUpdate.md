#IsotropicPowerLawHardeningStressUpdate
!syntax description /Materials/IsotropicPowerLawHardeningStressUpdate


## Description
!include docs/content/documentation/modules/tensor_mechanics/common/supplementalRadialReturnStressUpdate.md

This class models power law hardening by using the relation
$$
\sigma = \sigma_y + K \epsilon^n
$$
where $\sigma_y$ is the yield stress. This class solves for the yield stress as the intersection of the power law relation curve and Hooke's law:
$$
\epsilon_y = \frac{\sigma_y}{E} = \left( \frac{\sigma_y}{K} \right)^n
$$

where $\epsilon_y$ is the total strain at the yield point and the stress $\sigma_y$ is the von Mises stress. Parameters from the parent class, RecomputeRadialReturnIsotropicPlasticity, are suppressed to enable this class to solve for yield stress:

$$
\sigma_y = \left( \frac{E^n}{K} \right)^{1/(n-1)}
$$

!syntax parameters /Materials/IsotropicPowerLawHardeningStressUpdate

!syntax inputs /Materials/IsotropicPowerLawHardeningStressUpdate

!syntax children /Materials/IsotropicPowerLawHardeningStressUpdate
