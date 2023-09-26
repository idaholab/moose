# Step 3a - Adding volumetric heating

In Step 3, we added the time-derivative term to the heat equation, and showed
how including it allows for the solution of transient thermal problems. Some
thermal problems also involve a volumetric heat source, which could arise
due to a number of phenomena, including  exothermic chemical reactions, nuclear
fission, and resistive heating.

!listing modules/heat_conduction/tutorials/introduction/therm_step03a.i

## Input file

### `Kernels`

The only change necessary to include a volumetric heating term to this problem
is to add an additional block under `Kernels` for the volumetric heat source,
which is provided by the `HeatSource` kernel. The prescribed value here is a 
constant heat source prescribed as a heat source per unit volume. Options exist
for prescribing this using a function, which provides the flexibility to model
a wide variety of problems.

It may seem unusual that this is prescribed using a Kernel because this is similar
in nature to a boundary condition. However, volume integrals are prescribed as
Kernels in MOOSE, regardless of whether they are source terms or terms that
depend on a derivative of the solution.

!listing modules/heat_conduction/tutorials/introduction/therm_step03a.i block=Kernels

## Questions

Before running the model, consider how the solution should change with the
addition of volumetric heating.

[Click here for the answer.](heat_conduction/tutorials/introduction/answer03c.md)

Now go ahead and run the input and visualize the result to see if it matches
the behavior you would expect.

### Exploring parameters

Try changing the magnitude of the volumetric heating and see how that affects
the solution.

