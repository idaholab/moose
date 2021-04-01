# Mechanics Coupling

Coupled phase field and mechanics simulations require a MOOSE executable that combine the `phase_field` and `tensor_mechanics` modules. One such executable can be built under `moose/modules/combined`. That directory also contains a set of examples that are worth looking at.

Full coupling between *phase field* and *mechanics* goes both ways:

1. The *phase field* variables influence the *mechanics* properties
2. The *mechanics* state creates a free energy contribution that enters the *phase field* equations

## Mechanical properties

The mechanical properties of the system can (and will) be a function of the phase field variables in a tightly coupled simulation.

#### Elasticity tensor

Different phases (switched by a non-conserved order parameter) can have different elasticity tensors

- [`CompositeElasticityTensor`](/CompositeElasticityTensor.md optional=true) is a tensor that depends on phase field variables in an arbitrary way.

#### Eigenstrain (misfit strain, stress-free strain)

Different phases (switched by a non-conserved order parameter) can have different eigenstrains. This is used to simulate lattice mismatch between phases.

- [`ComputeVariableEigenstrain`](/ComputeVariableEigenstrain.md optional=true) is a tensor with a variable dependent scalar prefactor. It is best used to turn an eigenstrain on or off depending on a concentration variable.
- [`CompositeEigenstrain`](/CompositeEigenstrain.md optional=true) is an eigenstrain tensor built from multiple tensor contributions weighted by material properties.

## Elastic free energy

To couple the phase field equations with mechanics a contribution of the deformation energy (elastic energy) needs to enter the free energy density of the system. The phase field equations should be assembled using the `CahnHilliard`, `SplitCHParsed`, and `AllenCahn` [Function Material Kernels](phase_field/FunctionMaterialKernels.md) which all take the free energy as a [Function Material](/phase_field/FunctionMaterials.md).

- Define the *chemical* free energy using a [Function Material](/phase_field/FunctionMaterials.md).
- The [`ElasticEnergyMaterial`](/ElasticEnergyMaterial.md) will automatically compute the free energy density contribution using the local stresses and strains.
- Use the [`DerivativeSumMaterial`](/DerivativeSumMaterial.md) to sum the *chemical* and *elastic* free energy contributions to a total free energy (which is then passed to the kernels).
