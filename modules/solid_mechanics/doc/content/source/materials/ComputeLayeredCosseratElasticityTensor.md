# ComputeLayeredCosseratElasticityTensor

!syntax description /Materials/ComputeLayeredCosseratElasticityTensor

Both the elasticity tensor and the elasticity flexural rigidity tensors are simply set as constants computed from the
Young modulus and Poisson ratio of the layered materials, the thickness of the joints and the normal and shear stiffness of the joints.
The derivation can be found in the 3D layered Cosserat elasticity chapter of the theory manual.

Note that the elasticity tensor can still be made to vary in space and time using the [!param](/Materials/ComputeLayeredCosseratElasticityTensor/elasticity_tensor_prefactor) [Function](Functions/index.md) parameter. The elasticity rigidity tensor is constant.

!alert note
This object is part of the layered Cosserat mechanics model. See the theory manual  (at [solid_mechanics/doc/theory/cosserat.pdf](https://github.com/idaholab/moose/tree/next/modules/solid_mechanics/doc/theory/cosserat.pdf))
for more explanation.

!syntax parameters /Materials/ComputeLayeredCosseratElasticityTensor

!syntax inputs /Materials/ComputeLayeredCosseratElasticityTensor

!syntax children /Materials/ComputeLayeredCosseratElasticityTensor
