# ConstantsFromElasticityTensor

!syntax description /Materials/ConstantsFromElasticityTensor

## Description

This material extracts several of the elastic constants from an elasticity tensor: Young's modulus, Poisson's ratio, shear modulus, and bulk modulus. This is useful for debugging and for tracking the evolution of those properties over time from complex models. These properties are computed as scalar values, with names of `youngs_modulus_from_tensor`, `poissons_ratio_from_tensor`, `shear_modulus_from_tensor`, and `bulk_modulus_from_tensor`. The `_from_tensor` postfix is used to distinguish these from properties that may be computed by the objects that compute the tensors.

!alert! note title=Requires Isotropic Elasticity Tensor
This model can only be applied to isotropic elasticity tensors.
!alert-end!

!syntax parameters /Materials/ConstantsFromElasticityTensor

!syntax inputs /Materials/ConstantsFromElasticityTensor

!syntax children /Materials/ConstantsFromElasticityTensor
