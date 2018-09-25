# FileMesh

!syntax description /Mesh/FileMesh

## Support File Formats

The `FileMesh` is the default type for MOOSE and as the name suggests it reads the mesh from an external file. MOOSE
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

!syntax parameters /Mesh/FileMesh

!syntax inputs /Mesh/FileMesh

!syntax children /Mesh/FileMesh
