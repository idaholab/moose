# FileMeshGenerator

!syntax description /Mesh/FileMeshGenerator

## Supported File Formats

The `FileMeshGenerator` is the default type for MOOSE and as the name suggests it reads the mesh from an external file. MOOSE
supports reading and writing a large number of formats and could be extended to read more.

Reading with the `FileMeshGenerator`. These capabilities are inherited from the
libmesh file IO readers.

| Extension   | Description |
| :-          | :- |
| .e, .exd    | Sandia's ExodusII format |
| .bxt        | DynaIO |
| .cpr        | Checkpoint file |
| .fro        | ACDL's surface triangulation file |
| .mat        | Matlab triangular ASCII file |
| .msh        | GMSH ASCII file |
| .n, .nem    | Sandia's Nemesis format |
| .node, .ele; .poly | TetGen ASCII file |
| .inp        | Abaqus .inp format |
| .off        | OFF |
| .ucd        | AVS's ASCII UCD format |
| .unv        | I-deas Universal format |
| .xda, .xdr  | libMesh formats |
| .vtk, .pvtu | Visualization Toolkit |

Writing (using the [Outputs](syntax/Outputs/index.md) block). We list these formats here
for convenience if you are considering using MOOSE as a mesh file converter.

| Extension   | Description              | Output type |
| :-          | :-                       | :- |
| .e, .exd    | Sandia's ExodusII format | [Exodus.md] |
| .cpr        | Checkpoint file          | [Checkpoint.md] |
| .dat        | Tecplot ASCII file       | [Tecplot.md] |
| .gmv        | LANL's GMV (General Mesh Viewer) format | [GMVOutput.md] |
| .n, .nem    | Sandia's Nemesis format  | [Nemesis.md] |
| .plt        | Tecplot binary file      | [Tecplot.md] |
| .xda, .xdr  | libMesh formats          | [XDA.md] |
| .vtk, .pvtu | Visualization Toolkit    | [VTKOutput.md] |

These formats (for writing meshes) are supported by libmesh and could easily be added to MOOSE if needed:

| Extension     | Description                       |
| :-            | :-                       | :- |
| .fro          | ACDL's surface triangulation file |
| .mesh, .meshb | Medit |
| .msh          | GMSH ASCII file |
| .node, .ele; .poly | TetGen ASCII file |
| .ucd          | AVS's ASCII UCD format |


## Unsupported File Formats

These file formats are unsupported, however, using other tools they can be converted to supported formats.
In general, the mesh must respect the limitations of the target format for a successful conversion.

Tools offering conversion capabilities:

- [Paraview](https://www.paraview.org/)
- [meshio](https://pypi.org/project/meshio/2.3.5/), can also be installed with `mamba/conda`
- [em2ex](https://github.com/cpgr/em2ex)

The conversion capabilities to be able to read those files are summarized here for convenience:

| Extension   | Description | Conversion tool | Target format |
| :-          | :-          | :-              | :-     |
| .msh        | ANSYS msh   | meshio          | Exodus |
| .avs        | AVS-UCD     | meshio          | Exodus |
| .cgns       | CGNS        | meshio/Paraview | Exodus |
| .xml        | DOLFIN xml  | meshio          | Exodus |
| .case       | EnSight     | Paraview        | Exodus |
| .f3grid     | FLAC3D      | meshio          | Exodus |
| .grdecl     | Eclipse     | em2ex           | Exodus |
| .csv        | Leapfrog Geothermal | em2ex   | Exodus |
| .h5m        | H5M         | meshio          | Exodus |
| .mdpa       | Kratos/MDPA | meshio          | Exodus |
| .mesh, .meshb | Medit     | meshio          | Exodus |
| .med        | MED/Salome  | meshio          | Exodus |
| .bdf/.fem/.nas | Nastran  | meshio          | Exodus |
| .vol        | Netgen      | meshio          | Exodus |
|             | Neuroglancer | meshio         | Exodus |
| .obj        | OBJ         | meshio          | Exodus |
| .post, .dato | PERMAS     | meshio          | Exodus |
| .ply        | PLY         | meshio          | Exodus |
| .stl        | STL         | meshio          | Exodus |
| .svg        | SVG         | meshio          | Exodus |
| .su2        | SU2         | meshio          | Exodus |
| .ugrid      | UGRID       | meshio          | Exodus |
| .tin        | WKT TIN     | meshio          | Exodus |
| .xdmf, .xmf | XDMF        | meshio          | Exodus |

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
