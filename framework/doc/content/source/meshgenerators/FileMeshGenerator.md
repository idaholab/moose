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

## Extra element integer

When reading a mesh file in Sandia's ExodusII format, users can use parameter `exodus_extra_element_integers` to load elemental variables for setting extra element integers of the mesh.
The names of the extra element integers will be the same as the names of the
element variables in the mesh file.

## Exodus restart

This generator can also be used for
restarting variables from the Exodus file format. In order to indicate that the
mesh file can be used to restart variables, simply set the parameter
`use_for_exodus_restart = true`. The `initial_from_file_var` parameter must also
be set in the variables sub-block as described in [MooseVariableBase.md#restart]
in order to perform variable restart.

Additional documentation about restarting from Exodus may be found in the [restart-recovery page](restart_recover.md optional=True).

## Loading a split mesh

[Mesh splits](syntax/Mesh/splitting.md) usually do not require a `FileMeshGenerator`, they can be performed and loaded from the command line. The only use case for loading a split mesh using a `FileMeshGenerator` is to perform additional mesh generation on the split. For example, a 2D split mesh can be pre-split before extrusion to avoid ever having to load the full 3D mesh in serial.

To load a split mesh using the `FileMeshGenerator`, split your mesh as usual using the command line:

```
- mpirun -n 4 <executable> -i <input_file> --split-mesh 16 --split-file mesh_splitted
```

then load it with the `FileMeshGenerator` input parameter by using distributed meshes with the command line option:

```
- mpirun -n 4 <executable> -i <input_file> --distributed-mesh
```

Input file:

```
[fmg]
  type = FileMeshGenerator
  file = 'mesh_splitted.cpr'
[]
```

!syntax parameters /Mesh/FileMeshGenerator

!syntax inputs /Mesh/FileMeshGenerator

!syntax children /Mesh/FileMeshGenerator
