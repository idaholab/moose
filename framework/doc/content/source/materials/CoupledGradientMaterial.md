# CoupledGradientMaterial / ADCoupledGradientMaterial

!syntax description /Materials/CoupledGradientMaterial

This material provides the ability to create a material property from the gradient of a variable. In addition, the gradient can be scaled with a scalar material property allows for additional flexibility. This gradient material property can then be used in kernels such as [ADConservativeAdvection](/ADConservativeAdvection.md).

## Example Input File Syntax

In this example, `CoupledGradientMaterial` is used to define a gradient material from the variable u, which is then multiplied by a scalar.

!listing test/tests/materials/coupled_gradient_material/exact.i block=Materials/mat

!syntax parameters /Materials/CoupledGradientMaterial

!syntax inputs /Materials/CoupledGradientMaterial

!syntax children /Materials/CoupledGradientMaterial
