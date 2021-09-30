> Experiment with different settings for the mechanical properties of the sample
> and the applied loading. What happens if you drastically reduce the Young's
> modulus or increase the applied pressure. Is the simulation result still valid?

For example try setting a Young's modulus of `1e8` and rerunning the problem.
You should see substantial deformation of the block.

The current input is set up for small deformation mechanics. Large load relative
to the material stiffness will lead to large deformation, for which a finite
strain formulation needs to be used. Read about our incremental finite strain
formulation [here](ComputeFiniteStrain.md).

In the Tensor mechanics Master action this finite strain formulation can be
activated by setting the
[!param](/Modules/TensorMechanics/Master/TensorMechanicsAction/strain) parameter
to `FINITE`. Furthermore you need to swap out the corresponding stress calculator from [`ComputeLinearElasticStress`](ComputeLinearElasticStress.md) to [`ComputeFiniteStrainElasticStress`](ComputeFiniteStrainElasticStress.md).

Rerun the problem again with these changes and your modified Young's modulus of
`1e8`. You will observe that MOOSE struggles to converge this problem due to the
very large deformation. We will revisit this in a later exercise when we convert
the input over to automatic differentiation.
