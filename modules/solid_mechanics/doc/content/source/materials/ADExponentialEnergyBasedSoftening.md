# AD Exponential Energy Based Softening

!syntax description /Materials/ADExponentialEnergyBasedSoftening

## Description

The material `ADExponentialEnergyBasedSoftening` computes the reduced stress and stiffness
in the direction of a crack according to a exponential function. The computed
cracked stiffness ratio softens the tensile response of the material once the
principle stress exceeds the cracking stress threshold of the material.

For more details, refer to the non-AD version [ExponentialEnergyBasedSoftening](/ExponentialEnergyBasedSoftening.md).

!syntax parameters /Materials/ADExponentialSoftening

!syntax inputs /Materials/ADExponentialSoftening

!syntax children /Materials/ADExponentialSoftening

!bibtex bibliography

