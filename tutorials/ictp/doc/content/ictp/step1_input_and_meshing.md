# Step 1: Input and Meshing id=ictp_step1

!---

## Input

An input file defines a MOOSE simulation and follows a standardized syntax, the "hierarchical input text" (hit) format.

A basic MOOSE input file *typically* has six "blocks":

- `[Mesh]`: Define the geometry of the domain
- `[Variables]`: Define the unknown(s) of the problem
- `[Kernels]`: Define the equation(s) to solve
- `[BCs]`: Define the boundary condition(s) of the problem
- `[Executioner]`: Define how the problem will be solved
- `[Outputs]`: Define how the solution will be returned

There are caveats to the above and many other blocks exist. Shortcut syntax can also be created to simplify input.

A listing of all syntax can be found [here](syntax/index.md).

!---

## Simple Mesh Input

Start with the generation of a 2D uniform grid:

!listing ictp/step1-1_meshing_basic.i

The above defines a [`GeneratedMeshGenerator`](GeneratedMeshGenerator.md) named `gmg` with the parameters:

- `dim`: sets the dimension to $2$
- `nx`: which builds $10$ elements in the $x$-direction
- `ny`: which builds $10$ elements in the $y$-direction

!---

## Run: Simple Mesh Input

```bash
cardinal-opt -i step1-1_meshing.basic.i --mesh-only
```

- The argument `-i` specifies which input file(s) to run (multiple will be merged)
- The `--mesh-only` argument forces only mesh generation and output in Exodus format
- In the same folder, the file `step1-1_meshing_basic_out.e` will be created
- These output files are commonly inspected using Paraview

!---

## Result: Simple Mesh Input

!style halign=center
!media step1-1_mesh.png style=width:40%

!---

## MeshGenerator System

- The previous example utilized the MOOSE [`MeshGenerator`](Mesh/index.md) system
- It only utilized a single `MeshGenerator`, but multiple can be chained together using the `input` parameter
- All of our examples that follow (including the Cardinal tutorial) will utilize the [`MeshGenerator`](Mesh/index.md) system
- The [`Reactor`](reactor/index.md) module supports the systematic mesh generation of common reactor types

!--

## Loading Meshes From File

Meshes can also be loaded from file, of which we support a variety of formats like Exodus, GMSH, Nemesis, Tecplot, VTK, etc

```moose
[Mesh]
  [from_file]
    type = FileMeshGenerator
    file = foo.e
  []
[]
```

!--

## Useful Input Syntax

```moose
# Include the input at file.i here
!include file.i

value = true       # boolean
value = '1 2 3 4'  # vector -> [1, 2, 3, 4]
value = '1 2; 3 4' # vector-of-vectors -> [[1, 2], [3, 4]]

# Equivalent to [Mesh][gmg]
[Mesh/gmg]
  type = GeneratedMeshGenerator
  ...
[]

# Reference another value
value = 1.0
another_value = ${value}

# Perform basic arithmetic on another parameter (raidus = 0.5)
diameter = 1.0
radius = ${fparse diameter/2}
```

!--

## Concentric Circle Mesh

Let's generate a more interesting mesh, one that represents something like a fuel pin in 2D:

!listing ictp/step1-2_concentric_circle.i

!--

## Run: Concentric Circle Mesh

```bash
cardinal-opt -i step1-2_meshing.basic.i --mesh-only
```

!---

## Result: Concentric Circle Mesh Input

!style halign=center
!media step1-2_mesh.png style=width:40%
