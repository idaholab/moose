# MeshModifiers

The `MeshModifiers` system is designed to modify the [MooseMesh.md] during the
simulation.

!alert note
Please let us know on [GitHub Discussions](https://github.com/idaholab/moose/discussions)
how you are using the `MeshModifiers` system so we can include your techniques on this page!

## Interaction with other systems

### Declaring subdomains and boundaries in the Mesh block

Future subdomains and boundaries that will be created by a `MeshModifier` may be referred to in the input file for
kernels and boundary conditions that are not active on the first time step but will come in as soon as the subdomains/
boundaries are created. When they are parsed in the input file, they cause MOOSE to error because they do not
currently exist. To avoid these errors, they should be declared in the ([!param](/Mesh/FileMesh/add_subdomain_ids) / [!param](/Mesh/FileMesh/add_subdomain_names))
parameters for subdomains and ([!param](/Mesh/FileMesh/add_sideset_ids) / [!param](/Mesh/FileMesh/add_sideset_names))
parameters for boundaries.

### Skipping boundary conditions when the variable moves away from the boundary

Boundary conditions could be defined on a boundary, and as the mesh modifiers operate, the variable they apply
to may no longer be defined on this boundary. The [!param](/BCs/NeumannBC/skip_execution_outside_variable_domain)
parameter of integrated boundary conditions lets the boundary become inactive after the variable has left, partially or totally, the proximity
of the boundary.
For Dirichlet boundary conditions, no parameter is needed to stop the effect of the boundary condition once the variable is no longer
defined near the boundary.

The boundary conditions will become active again if the domain of definition of the variable extends again to touch the boundary.

!syntax list /MeshModifiers objects=True actions=False subsystems=False

!syntax list /MeshModifiers objects=False actions=False subsystems=True

!syntax list /MeshModifiers objects=False actions=True subsystems=False
