# AD Abrupt Softening

!syntax description /Materials/ADAbruptSoftening

## Description

The material `ADAbruptSoftening` computes the reduced stress and stiffness
in the direction of a crack according to a step function. The computed
cracked stiffness ratio softens the tensile response of the material once the
principle stress exceeds the cracking stress threshold of the material. 
It uses automatic differentiation for the computation of its Jacobian. 
For more details, refer to the non-AD version [`AbruptSoftening`](/AbruptSoftening.md).


## Example Input File

!listing modules/tensor_mechanics/test/tests/ad_smeared_cracking/cracking_rz.i block=Materials/abrupt_softening

`ADAbruptSoftening` must be run in conjunction with the fixed ad smeared cracking material model as shown below:

!listing modules/tensor_mechanics/test/tests/ad_smeared_cracking/cracking_rz.i block=Materials/elastic_stress

!syntax parameters /Materials/ADAbruptSoftening

!syntax inputs /Materials/ADAbruptSoftening

!syntax children /Materials/ADAbruptSoftening

!bibtex bibliography
