# Compute Plane Stress Isotropic Elasticity Tensor Material

## Description

Material `ComputePlaneStressIsotropicElasticityTensor` converts the 3D isotropic elasticity tensor to one specifically for plane stress case. All other unrelated components in the tensor are set to zeros.

Note that Material `ComputePlaneStressIsotropicElasticityTensor` can be used +ONLY+ for elastic materials.

!syntax parameters /Materials/ComputePlaneStressIsotropicElasticityTensor

!syntax inputs /Materials/ComputePlaneStressIsotropicElasticityTensor

!syntax children /Materials/ComputePlaneStressIsotropicElasticityTensor
