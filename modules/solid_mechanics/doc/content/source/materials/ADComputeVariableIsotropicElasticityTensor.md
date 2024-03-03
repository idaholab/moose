# ADCompute Variable Isotropic Elasticity Tensor

!syntax description /Materials/ADComputeVariableIsotropicElasticityTensor

## Description

This model computes an elasticity tensor for which the elastic moduli, prescribed in terms of the
Young's modulus and Poisson's ratio, vary as defined by user-specified material properties. Every
time this material is evaluated, the full tensor is updated to reflect the current values of those
elastic constants.

## Example Input File Syntax

!listing modules/combined/test/tests/thermal_elastic/ad-thermal_elastic.i block=Materials/elasticity_tensor

!syntax parameters /Materials/ADComputeVariableIsotropicElasticityTensor

!syntax inputs /Materials/ADComputeVariableIsotropicElasticityTensor

!syntax children /Materials/ADComputeVariableIsotropicElasticityTensor
