# [Mesh System](syntax/Mesh/index.md)

A system for defining a finite element / volume mesh.

!---

## Creating a Mesh

For complicated geometries, we often use CUBIT from Sandia National Laboratories
[cubit.sandia.gov](https://cubit.sandia.gov).

Other mesh generators can work as long as they output a file format that libMesh reads.

!---

## Mesh generators

Meshes in MOOSE are built or loaded using [MeshGenerators](syntax/Mesh/index.md).

To only generate the mesh without running the simulation, you can pass `--mesh-only` on the command line.

!---

## FileMeshGenerator

`FileMeshGenerator` is the MeshGenerator to load external meshes:

!listing test/tests/meshgenerators/file_mesh_generator/file_mesh_generator.i block=Mesh

MOOSE supports reading and writing a large number of formats and could be extended to read more.

!---

| Extension   | Description                              |
| :-          | :-                                       |
| .dat        | Tecplot ASCII file                       |
| .e, .exd    | Sandia's ExodusII format                 |
| .fro        | ACDL's surface triangulation file        |
| .gmv        | LANL's GMV (General Mesh Viewer) format  |
| .mat        | Matlab triangular ASCII file (read only) |
| .msh        | GMSH ASCII file                          |
| .n, .nem    | Sandia's Nemesis format                  |
| .plt        | Tecplot binary file (write only)         |
| .node, .ele; .poly | TetGen ASCII file (read; write)   |
| .inp        | Abaqus .inp format (read only)           |
| .ucd        | AVS's ASCII UCD format                   |
| .unv        | I-deas Universal format                  |
| .xda, .xdr  | libMesh formats                          |
| .vtk, .pvtu | Visualization Toolkit                    |

!---

## Generating Meshes in MOOSE

Built-in mesh generation is implemented for lines, rectangles,  rectangular prisms or [extruded reactor geometries](modules/reactor/index.md).

!style! fontsize=50%

!listing initial_adaptivity_test.i block=Mesh

!style-end!

The sides are named in a logical way and are numbered:

- 1D: left = 0, right = 1
- 2D: bottom = 0, right = 1, top = 2, left = 3
- 3D: back = 0, bottom = 1, right = 2, top = 3, left = 4, front = 5

!---

## Mini-meshing hands-on

Let's mesh the "flow over circle" geometry, used for a computational fluid dynamics benchmark

!media flow-over-circle.png style=width:60%;margin-left:auto;margin-right:auto;display:block;

!---

Two inputs are combined on the command line with `exec-opt -i header.i mesh.i`

!style! fontsize=50%

!listing modules/navier_stokes/examples/flow-over-circle/header.i end=Material max-height=250px

!listing modules/navier_stokes/examples/flow-over-circle/mesh.i start=[Mesh] end=middle_top_sideset max-height=250px

!style-end!

!---

## Replicated Mesh

When running in parallel the default mode for operation is to use a replicated mesh, which
creates a complete copy of the mesh for each processor.

```text
parallel_type = replicated
```

!---

## Distributed Mesh

Changing the type to distributed when running in parallel operates such that only the portion of the
mesh owned by a processor is stored on that processor.

```text
parallel_type = distributed
```

If the mesh is too large to read in on a single processor, it can be split prior to the simulation.

1. Copy the mesh to a large memory machine
1. Use the `--split-mesh` option to split the mesh into $n$ pieces
1. Run the executable with `--use-split`
