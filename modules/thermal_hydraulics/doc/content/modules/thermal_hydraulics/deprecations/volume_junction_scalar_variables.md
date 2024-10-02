# Volume Junction Scalar Variables

## Background

Several components inherit from `VolumeJunctionBase`:

- [JunctionParallelChannels1Phase.md]
- [Pump1Phase.md]
- [ShaftConnectedCompressor1Phase.md]
- [ShaftConnectedPump1Phase.md]
- [ShaftConnectedTurbine1Phase.md]
- [SimpleTurbine1Phase.md]
- [VolumeJunction1Phase.md]

Here, these components will be referred to as "junctions".
Before, these components all created *scalar* solution variables for various
quantities. However, it was found that this was extremely costly due to the
sparsity pattern requirements, thus making simulations involving large numbers
of these components to be very slow to initialize. A new option,
[!param](/Components/VolumeJunction1Phase/use_scalar_variables), has been added
to these components, with a default value of `true`, resulting in the old behavior.
The new behavior, enabled by setting this parameter to `false`, is to create
*field* variables instead of scalar variables, with a single node and node-element added to the
mesh for each of these components added to the simulation, and a single degree
of freedom per variable. Mathematically, the old and new implementations are
equivalent. Additionally, it was found that having large numbers of unique
variable names was also detrimental to performance, so this change also changes
the naming of junction variables: before, junction variables were named to
include the name of the component, e.g., `junction1:rhoV`. Now, since these
variables are tied to the mesh, they can have a common name, e.g., `rhoV`.

!alert tip title=Visualizing junction solutions
Since these variables are now located on the mesh, they can visualized alongside
the other field variables, such as from pipes. In Paraview, the junction can be
made more visible using the "point size" parameter.

## Instructions

The old behavior is now deprecated and will be removed in the near
future. To switch to the new behavior, set
[!param](/Components/VolumeJunction1Phase/use_scalar_variables)
to `false` and note the following additional changes that may be necessary:

- If references to junction variable names were made in the input file,
  these will need to be updated to remove the component name. For example,
  the variable name `component1:rhoV` should become `rhoV`.
- If you have regression tests and see Exodiff failures, it is because the mesh
  is now different, since it has additional mesh nodes and elements for the
  junctions, so "gold" files will need to be updated.
