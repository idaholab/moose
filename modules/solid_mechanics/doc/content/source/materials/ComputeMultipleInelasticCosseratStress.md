# ComputeMultipleInelasticCosseratStress

!syntax description /Materials/ComputeMultipleInelasticCosseratStress

The `ComputeMultipleInelasticCosseratStress` performs the same functions as the [ComputeMultipleInelasticStress.md]
material, but additionally computes:

- the coupled Cosserat elastic stress $\boldsymbol{\sigma}_c$
- the derivative tensor of the coupled Cosserat elastic stress with regards to strain

These two terms are computed in the same way as [ComputeCosseratLinearElasticStress.md].
If finite strain rotations are considered, then the these two terms are further modified with the rotation increment $R$:

!equation
\boldsymbol{\sigma}_c := R \boldsymbol{\sigma}_c R^T

The derivative tensor of the coupled Cosserat elastic stress with regards to strain is rotated by the rotation increment.

!alert note
This object is part of the Cosserat mechanics model. See the theory manual (at [solid_mechanics/doc/theory/cosserat.pdf](https://github.com/idaholab/moose/tree/next/modules/solid_mechanics/doc/theory/cosserat.pdf))
for more explanation.

!syntax parameters /Materials/ComputeMultipleInelasticCosseratStress

!syntax inputs /Materials/ComputeMultipleInelasticCosseratStress

!syntax children /Materials/ComputeMultipleInelasticCosseratStress
