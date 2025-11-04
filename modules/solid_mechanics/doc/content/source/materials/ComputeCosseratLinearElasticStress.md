# ComputeCosseratLinearElasticStress

!syntax description /Materials/ComputeCosseratLinearElasticStress

The Cosserat linear elastic stress tensor is computed from the Cosserat elasticity tensor, within the small strain approximation:

!equation
\boldsymbol{\sigma} = \boldsymbol{E} \boldsymbol{\gamma}

!equation
\boldsymbol{\sigma}_c = \boldsymbol{B} C

where

- $\boldsymbol{\sigma}$ is the linear elastic stress tensor
- $\boldsymbol{E}$ the elasticity tensor (for example computed by [ComputeCosseratElasticityTensor.md])
- $\boldsymbol{\gamma}$ the strain tensor (for example computed by [ComputeCosseratSmallStrain.md])
- $\boldsymbol{\sigma}_c$ the coupled elastic stress tensor
- $\boldsymbol{B}$ the elastic flexural rigidity tensor
- C the local curvature tensor (for example computed by [ComputeCosseratSmallStrain.md])

The elastic strain is also computed to be the same as the mechanical strain, as there is no plasticity.
The Jacobian, derivative of the stress with regards to strain, is simply set to the elasticity tensor,
and the derivative of the coupled stress with regards to strain, is simply set to the elasticity flexural rigidity tensor.

!alert note
This object is part of the Cosserat mechanics model. See the theory manual (at [solid_mechanics/doc/theory/cosserat.pdf](https://github.com/idaholab/moose/tree/next/modules/solid_mechanics/doc/theory/cosserat.pdf))
for more explanation.

!syntax parameters /Materials/ComputeCosseratLinearElasticStress

!syntax inputs /Materials/ComputeCosseratLinearElasticStress

!syntax children /Materials/ComputeCosseratLinearElasticStress
