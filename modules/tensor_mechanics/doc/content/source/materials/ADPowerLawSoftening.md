# AD Power Law Softening

!syntax description /Materials/ADPowerLawSoftening

## Description

The material `ADPowerLawSoftening` computes the reduced stress and stiffness along
the direction of a crack according to a power law equation. The computed
reduced stiffness softens the tensile response of the material once the principle
stress applied to a material exceeds the cracking stress threshold of the material. 
It uses automatic differentiation for the computation of its Jacobian.
As with the other smeared cracking softening models, which all follow the
nomenclature convention of using the `Softening` suffix, this model is intended
to be used with the [ADComputeSmearedCrackingStress](/ADComputeSmearedCrackingStress.md)
material.

For more details, refer to the non-AD class [PowerLawSoftening](/PowerLawSoftening.md).

## Example Input File

!listing modules/tensor_mechanics/test/tests/smeared_cracking/cracking_power.i block=Materials/power_law_softening

`ADPowerLawSoftening` must be run in conjunction with the ad fixed smeared cracking material model as shown below:

!listing modules/tensor_mechanics/test/tests/ad_smeared_cracking/cracking_power.i block=Materials/elastic_stress

!syntax parameters /Materials/ADPowerLawSoftening

!syntax inputs /Materials/ADPowerLawSoftening

!syntax children /Materials/ADPowerLawSoftening

!bibtex bibliography
