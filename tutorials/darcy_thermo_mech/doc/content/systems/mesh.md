# Mesh System

A system for defining a finite element mesh.

!---

## Creating a Mesh

For complicated geometries, we generally use CUBIT from Sandia National Laboratories
[cubit.sandia.gov](https://cubit.sandia.gov).

Other mesh generators can work as long as they output a file format that libMesh reads.

!---

## FileMesh

`FileMesh` is the default type:

!listing mesh/gmsh/gmsh_test.i

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

## GeneratedMesh

Built-in mesh generation is implemented for lines, rectangles, and rectangular prisms

!listing initial_adaptivity_test.i block=Mesh

The sides are named in a logical way and are numbered:

- 1D: left = 0, right = 1
- 2D: bottom = 0, right = 1, top = 2, left = 3
- 3D: back = 0, bottom = 1, right = 2, top = 3, left = 4, front = 5

!---

## Named Entity Support

Human-readable names can be assigned to blocks, sidesets, and nodesets that can be used throughout
an input file.

A parameter that requires an ID will accept either numbers or "names".

Names can be assigned to IDs for existing meshes to ease input file maintenance.

!---

!listing name_on_the_fly.i block=Mesh BCs Materials

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

!---

## Displaced Mesh

Calculations can take place in either the initial mesh configuration or, when requested, the
"displaced" configuration.

To enable displacements, provide a vector of displacement variable names for each spatial dimension
in the Mesh block.

!listing dg_displacement.i block=Mesh

Objects can enforce the use of the displaced mesh within the validParams function.

!listing PenetrationAux.C line=use_displaced_mesh
