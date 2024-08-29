# ComputeCosseratElasticityTensor

!syntax description /Materials/ComputeCosseratElasticityTensor

Both the elasticity tensor and the elasticity flexural rigidity tensors are simply set as constants through the
[!param](/Materials/ComputeCosseratElasticityTensor/E_ijkl) and [!param](/Materials/ComputeCosseratElasticityTensor/B_ijkl)
parameters respectively.

See [ComputeElasticityTensor.md] for an explanation about the fill methods, e.g. how to input a tensor in a parameter.

Note that the elasticity tensor can still be made to vary in space and time using the [!param](/Materials/ComputeCosseratElasticityTensor/elasticity_tensor_prefactor)
[Function](Functions/index.md) parameter. The elasticity rigidity tensor is constant.

!alert note
This object is part of the Cosserat mechanics model. See the theory manual  (at [solid_mechanics/doc/theory/cosserat.pdf](https://github.com/idaholab/moose/tree/next/modules/solid_mechanics/doc/theory/cosserat.pdf))
for more explanation.

!syntax parameters /Materials/ComputeCosseratElasticityTensor

!syntax inputs /Materials/ComputeCosseratElasticityTensor

!syntax children /Materials/ComputeCosseratElasticityTensor
