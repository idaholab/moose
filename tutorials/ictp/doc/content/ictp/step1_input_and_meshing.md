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

!listing moose/step1-1_meshing_basic.i

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
- It only utilized a single `MeshGenerator`, but multiple can be chained together (examples to follow)
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

## Fuel Pin Mesh

The grid we've generated so far isn't particularly interesting.

Let's generate a mesh that represents represents a fuel pin surrounded by water in a square lattice using the [`ConcentricCircleMeshGenerator`](ConcentricCircleMeshGenerator.md).

!--

## Concentric Circle Mesh

!listing moose/step1-2_concentric_circle.i

!--

## Run: Concentric Circle Mesh

```bash
cardinal-opt -i step1-2_concentric_circle.i --mesh-only
```

!---

## Result: Concentric Circle Mesh Input

!style halign=center
!media step1-2_mesh.png style=width:25%

This mesh has five blocks (also called subdomains):

- $1$: The inner hole (grey)
- $2$: The fuel meat (red)
- $3$: The cladding (green)
- $4$: The water boundary layer (blue)
- $5$: The remaining water (yellow)

!---

## Separating Meshes

Later in this tutorial, we will be treating the fuel pin and the surrounding fluid independently. Thus, we will now take the mesh that we just generated and separate it into two components: one for the fuel pin and one for the fluid.

This task reveals a capability of MeshGenerators - they can be combined together. Some MeshGenerators generate meshes, some modify meshes, and some do both. Oftentimes a generator will have an input parameter called `input`, which is used as the name of another MeshGenerator to be used as input to modify.

For the tasks that follow, we will utilize the:

- [`BlockDeletionGenerator`](BlockDeletionGenerator.md): Removes blocks from a mesh
- [`RenameBlockGenerator`](RenameBlockGenerator.md): Renames and optionally merges blocks on a mesh
- [`RenameBoundaryGenerator`](RenameBoundaryGenerator.md): Renames and optionally merges boundaries on a mesh

!--

## Fuel Mesh

- Remove the water elements using the [`BlockDeletionGenerator`](BlockDeletionGenerator.md)
- Remove the hole (assume no net heat transfer across the hole) using the [`BlockDeletionGenerator`](BlockDeletionGenerator.md)
- Name the blocks that contain the fuel and cladding using the [`RenameBlockGenerator`](RenameBlockGenerator.md)

!--

## Input: Fuel Mesh

!listing moose/step1-3_fuel_pin.i

!--

## Run: Fuel Mesh

Here, we add a value to `--mesh-only` that stores the resulting mesh in `fuel_pin.e`:

```bash
cardinal-opt -i step1-3_fuel_pin.i --mesh-only fuel_pin.e
```

!---

## Result: Fuel Mesh

!style halign=center
!media step1-3_mesh.png style=width:35%

The grey elements represent the fuel (block `fuel`), and the red elements represent the cladding (block `clad`). There is a sideset on the outer boundary with name `outer` and sideset on the inner boundary with the name `inner`.

!---

## Fluid Mesh

- Remove the fuel pin elements using the [`BlockDeletionGenerator`](BlockDeletionGenerator.md)
- Merge the water boundary layer and remaining water blocks into a single block named `water` using the [`RenameBlockGenerator`](RenameBlockGenerator.md)
- Rename and merge the outer boundaries into a single boundary `outer` using the [`RenameBoundaryGenerator`](RenameBoundaryGenerator.md)

!---

## Input: Fluid Mesh

!listing moose/step1-4_fluid.i

!---

## Run: Fluid Mesh

Generate the mesh and output it to `fluid.e`:

```bash
cardinal-opt -i step1-4_fluid.i --mesh-only fluid.e
```

!---

## Result: Fluid Mesh

!style halign=center
!media step1-4_mesh.png style=width:35%

The grey elements represent the water (block `water`). There is a sideset on the inner boundary with the name `inner` and a sideset on the outer boundary with the name `outer`.

!---

## Re-using The Meshes

We can then load these meshes in later inputs with:

```moose
[Mesh/fuel_pin]
  type = FileMeshGenerator
  file = fuel_pin.e
[]
```

```moose
[Mesh/fluid]
  type = FileMeshGenerator
  file = fluid.e
[]
```
