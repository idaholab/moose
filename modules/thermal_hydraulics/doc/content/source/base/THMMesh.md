# THMMesh

!syntax description /Mesh/THMMesh

The `THMMesh` is created by combining the meshes of all the components.
It provides APIs to add edge (1D) and quadrilateral (2D) elements which the components call
to build the mesh.

The `THMMesh` can be set to second-order by setting the `Problem` parameter [!param](/Problem/THMProblem/2nd_order_mesh) to true.
This is performed automatically for second order variables.

!alert note
The THMMesh is created automatically by the [Simulation.md]. The user does not need to create it in their input
file.

!syntax parameters /Mesh/THMMesh

!syntax inputs /Mesh/THMMesh

!syntax children /Mesh/THMMesh
