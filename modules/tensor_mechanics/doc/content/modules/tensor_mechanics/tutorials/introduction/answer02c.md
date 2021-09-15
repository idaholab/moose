> If you created a large strain version of the input, try and convert it to use
> MOOSE's automatic differentiation system. A few places to look at:
>
> - [!param](/Modules/TensorMechanics/Master/TensorMechanicsAction/use_automatic_differentiation) in the tensor mechanics master action
> - [!param](/BCs/Pressure/PressureAction/use_automatic_differentiation) in the Pressure BC action
> - [ADDirichletBC](ADDirichletBC.md)
> - [ADComputeIsotropicElasticityTensor](ComputeIsotropicElasticityTensor.md)
> - [ADComputeFiniteStrainElasticStress](ADComputeFiniteStrainElasticStress.md)

Here is the converted input:
