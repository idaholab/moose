# ComputeDSGShellStrain

!syntax description /Materials/ComputeDSGShellStrain

## Description

This material computes the generalized strains of a flat shell element of the discrete shear gap (DSG) family, with three, four, six or eight nodes and six degrees of freedom per node. It reads the three displacement and the three rotation variables of the element, builds the element frame from the node coordinates, assembles the strain-displacement matrix of the whole element and stores, at every quadrature point, the matrix, the generalized strains, the rotation matrix of the element frame and the longest edge. The formulation is described in [DSGShellElements](/DSGShellElements.md).

The displacement and rotation variables must be Lagrange variables of the order of the element: `FIRST` on `TRI3` and `QUAD4` meshes, `SECOND` on `TRI6` and `QUAD8` meshes. Any other element type, or a variable order that does not match the mesh, stops the simulation with a message naming the element.

The first local axis of the element frame is the projection of `reference_first_local_direction` on the element plane, as for the module's four-node shell. When that direction is normal to the element (within 0.05 degrees), the global axis least aligned with the normal is projected instead.

With `compute_nodal_strains = true` the material also evaluates the generalized strains at the nodes of each element, from the same strain-displacement matrix at the parent coordinates of the node, and stores them in `dsg_shell_nodal_generalized_strain` (one row per node) for the nodal recovery of the resultants by [DSGShellNodalResultants](/DSGShellNodalResultants.md).

### Choosing the parameters

- `displacements` and `rotations` are the six nodal variables, in the order of the global axes; every object of the family takes the same two lists.
- `reference_first_local_direction` fixes the direction 1 of the resultants on each element: its projection on the element plane is the first local axis. The default, the global X axis, suits plates in the XY plane. On a curved surface choose a direction that is not normal to any element, for example the axis of a cylinder, so that direction 1 is the axial direction everywhere and direction 2 the hoop direction.
- `compute_nodal_strains` is off by default and is only needed when the resultants are recovered at the nodes with [DSGShellNodalResultants](/DSGShellNodalResultants.md); it adds one strain evaluation per node to the assembly of every element.

## Example Input File syntax

!listing modules/solid_mechanics/test/tests/shell_dsg/static/cantilever_strip.i block=Materials

!syntax parameters /Materials/ComputeDSGShellStrain

!syntax inputs /Materials/ComputeDSGShellStrain

!syntax children /Materials/ComputeDSGShellStrain
