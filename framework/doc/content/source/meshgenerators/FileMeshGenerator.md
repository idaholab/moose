# FileMeshGenerator

!syntax description /Mesh/FileMeshGenerator

## Supported File Formats

The `FileMeshGenerator` is the default type for MOOSE and as the name suggests it reads the mesh from an external file. MOOSE
supports reading and writing a large number of formats and could be extended to read more.


| Extension   | Description |
| :-          | :- |
| .e, .exd    | Sandia's ExodusII format |
| .dat        | Tecplot ASCII file |
| .fro        | ACDL's surface triangulation file |
| .gmv        | LANL's GMV (General Mesh Viewer) format |
| .mat        | Matlab triangular ASCII file (read only) |
| .msh        | GMSH ASCII file |
| .n, .nem    | Sandia's Nemesis format |
| .plt        | Tecplot binary file (write only) |
| .node, .ele; .poly | TetGen ASCII file (read; write) |
| .inp        | Abaqus .inp format (read only) |
| .ucd        | AVS's ASCII UCD format |
| .unv        | I-deas Universal format |
| .xda, .xdr  | libMesh formats |
| .vtk, .pvtu | Visualization Toolkit |
| .cpr        | Checkpoint file |

When reading a mesh file in Sandia's ExodusII format, users can use parameter `exodus_extra_element_integers` to load elemental variables for setting extra element integers of the mesh.
The names of the extra element integers will be the same as the names of the
element variables in the mesh file. This generator can also be used for
restarting variables from the Exodus file format. In order to indicate that the
mesh file can be used to restart variables, simply set the parameter
`use_for_exodus_restart = true`. The `initial_from_file_var` parameter must also
be set in the variables sub-block as described in [MooseVariableBase.md#restart]
in order to perform variable restart.

## Further FileMeshGenerator Documentation

!syntax parameters /Mesh/FileMeshGenerator

!syntax inputs /Mesh/FileMeshGenerator

!syntax children /Mesh/FileMeshGenerator
