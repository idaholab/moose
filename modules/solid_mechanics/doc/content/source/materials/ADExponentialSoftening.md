# AD Exponential Softening

!syntax description /Materials/ADExponentialSoftening

## Description

The material `ADExponentialSoftening` computes the reduced stress and stiffness
in the direction of a crack according to a exponential function. The computed
cracked stiffness ratio softens the tensile response of the material once the
principle stress exceeds the cracking stress threshold of the material. 
It uses automatic differentiation for the computation of its Jacobian.

For more details, refer to the non-AD version [ExponentialSoftening](/ExponentialSoftening.md).

## Example Input File

!listing modules/tensor_mechanics/test/tests/ad_smeared_cracking/cracking_rotation.i block=Materials/exponential_softening

`ADExponentialSoftening` must be run in conjunction with the ad fixed smeared cracking material model as shown below:

!listing modules/tensor_mechanics/test/tests/ad_smeared_cracking/cracking_rotation.i block=Materials/cracking_stress

!syntax parameters /Materials/ADExponentialSoftening

!syntax inputs /Materials/ADExponentialSoftening

!syntax children /Materials/ADExponentialSoftening

!bibtex bibliography
