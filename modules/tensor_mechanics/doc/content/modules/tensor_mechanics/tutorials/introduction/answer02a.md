> Experiment with different settings for the mechanical properties of the sample
> and the applied loading. What happens if you drastically reduce the Young's
> modulus or increase the applied pressure. Is the simulation result still valid?

The current input is set up for small deformation mechanics. Large load relative
to the material stiffness will lead to large deformation, for which a finite
strain formulation needs to be used. Read about our incremental finite strain
formulation [here](ComputeFiniteStrain.md).

In the Tensor mechanics Master action this finite strain formulation it can be
activated by setting the
[!param](/Modules/TensorMechanics/Master/TensorMechanicsAction/strain) parameter
to `FINITE`. Furthermore you need to swap out the corresponding stress calculator from [`ComputeLinearElasticStress`](ComputeLinearElasticStress.md) to [`ComputeFiniteStrainElasticStress`](ComputeFiniteStrainElasticStress.md).

Give that try!
