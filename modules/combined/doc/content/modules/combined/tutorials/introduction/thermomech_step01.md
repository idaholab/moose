# Step 1 - Basic Thermal/Mechanical Coupling

In the tutorials for the Heat Conduction and Tensor Mechanics modules,
basic thermal and mechanical problems were developed. The Heat Conduction tutorial
culminated with a [model](heat_conduction/tutorials/introduction/therm_step03a.md)
that solves the heat equation on a simple rectangular domain, including terms
for heat conduction, time-dependent effects, and volumetric heating.

The Tensor Mechanics tutorial includes a [model](tensor_mechanics/tutorials/introduction/step03a.md)
that shows how thermal expansion can be accounted for using a prescribed
temperature field.

The model shown here builds on these thermal and mechanical models by simultaneously
solving for the thermal and mechanical response. As will be evident from examining this file,
coupled physics models are defined by combining the components from all of the physics
in a single input file, and defining interactions between the physics.

!listing modules/combined/tutorials/introduction/thermal_mechanical/thermomech_step01.i

## Input file

### `Variables`

The `Variables` block in this input files looks just like it does for a thermal-only
problem. A single variable,`T`, is defined, with an initial condition of 300 Kelvin.
An obvious question is why no variables are defined for mechanics. Coupled physics 
problems are usually defined in MOOSE by defining multiple variables and their associated
Kernels. This is actually done in this model, but the `Modules/TensorMechanics/Master` Action
block automatically sets up the displacement variables, so they don't explicitly 
appear in the `Variables` block in the input file. That Action sets up variables
with the names defined by the `displacements` parameter, which is defined in
`GlobalParams`. In this case, there are thus three solution variables: `T`, `disp_x`,
  and `disp_y`.

!listing modules/combined/tutorials/introduction/thermal_mechanical/thermomech_step01.i block=Variables

### `Kernels`

Similar to the `Variables` block, only the kernels related to the temperature solution
are defined in the `Kernels` block in this case. This is again due to the fact
that the `Modules/TensorMechanics/Master` Action automates setting up the Kernels associated
with the displacement variables, so they don't explicitly appear in the input file.
This model includes kernels for the conduction, time derivative, and volumetric terms
in the heat equation.

!listing modules/combined/tutorials/introduction/thermal_mechanical/thermomech_step01.i block=Kernels

### `Modules/TensorMechanics/Master`

This block automates the process of setting up multiple objects related to solution of
a mechanics problem. The objects set up by this block include the Variables and
Kernels for the displacement solution, the Material that computes the strain, and
objects associated with outputing stresses. In addition to simplifying the input,
this ensures that a consistent set of options are selected for the desired formulation.

!listing modules/combined/tutorials/introduction/thermal_mechanical/thermomech_step01.i block=Modules/TensorMechanics/Master/all

### `Materials`

Material properties must be defined for both the thermal and mechanical models. 
For the thermal model, the thermal conductivity, specific heat, and density
are needed. For the mechanical model, the elasticity tensor, thermal eigenstrain, and stress
are computed. The models listed here are essentially a combination of the models
that were used for the single-physics thermal and mechanical models.

!listing modules/combined/tutorials/introduction/thermal_mechanical/thermomech_step01.i block=Materials

### `BCs`

Boundary conditions must be prescribed for the individual physics in the same way
that they are for single-physics models. The `variable` parameter in each of these
boundary condition blocks defines the variable that the boundary condition is
applied to.

!listing modules/combined/tutorials/introduction/thermal_mechanical/thermomech_step01.i block=BCs

### `Preconditioning`

The `Preconditioning` block is used here to define that a full matrix with coupling
between all variables is used for preconditioning. This is more memory intensive
than the default block-diagonal matrix, but typically results in improved convergence
for multiphysics models.

The remaining blocks (`Executioner` and `Outputs`) are the same as they would be for
single-physics models in this case.


## Questions

Where is coupling introduced between the thermal and mechanical solutions in
this model?

[Click here for the answer.](combined/tutorials/introduction/thermomech_answer01.md)

Once you've answered the questions and run this example we will move on to
[Step 2](combined/tutorials/introduction/step02.md) in which we introduce
a third block participating in the contact problem.
