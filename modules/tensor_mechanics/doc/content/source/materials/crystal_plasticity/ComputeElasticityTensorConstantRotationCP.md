# ComputeElasticityTensorConstantRotationCP

!syntax description /Materials/ComputeElasticityTensorConstantRotationCP

## Description

The material `ComputeElasticityTensorConstantRotationCP` is used to create a constant
elasticity tensor for crystal plasticity simulations which refer only to the reference
configuration. As such, this class only rotates the elastic tensor once, during
the initial setup step.

!alert warning
This class is deprecated. Please use the [ComputeElasticityTensorCP](/ComputeElasticityTensorCP.md) class instead.

This material builds an orthotropic elasticity tensor using the fill_method
`symmetric9` from [ComputeElasticityTensor](/ComputeElasticityTensor.md).


!alert warning title=Crystal Plasticity Simulations use Active Rotation
The rotation matrix used in this class,`ComputeElasticityTensorConstantRotationCP`, is the transpose of the rotation matrix created from the Bunge Euler angles in the base class,
[ComputeElasticityTensor](/ComputeElasticityTensor.md). This difference in the
rotation matrix is because of the active rotation convention used in crystal
plasticity simulations.

!syntax parameters /Materials/ComputeElasticityTensorConstantRotationCP

!syntax inputs /Materials/ComputeElasticityTensorConstantRotationCP

!syntax children /Materials/ComputeElasticityTensorConstantRotationCP

!bibtex bibliography
