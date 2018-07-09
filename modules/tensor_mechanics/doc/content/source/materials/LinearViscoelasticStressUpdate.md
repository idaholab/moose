# Linear Viscoelastic Stress Update

!syntax description /Materials/LinearViscoelasticStressUpdate

## Description

This computes the inelastic strain increment resulting from a linear viscoelastic material such as a [GeneralizedKelvinVoigtModel](/GeneralizedKelvinVoigtModel.md) or a [GeneralizedMaxwellModel](/GeneralizedMaxwellModel.md) material. It uses an incremental strain approximation (either incremental small strains, or finite strains), and needs to be used in conjunction with [ComputeMultipleInelasticStress](/ComputeMultipleInelasticStress.md) or a similar stress calculator.

## Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/visco/gen_maxwell_driving.i block=Materials/creep

`LinearViscoelasticStressUpdate` must be run in conjunction with the inelastic strain return mapping stress calculator as shown below:

!listing modules/tensor_mechanics/test/tests/visco/gen_maxwell_driving.i block=Materials/stress

!syntax parameters /Materials/LinearViscoelasticStressUpdate

!syntax inputs /Materials/LinearViscoelasticStressUpdate

!syntax children /Materials/LinearViscoelasticStressUpdate
