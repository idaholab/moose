# Step 1: Input and Meshing id=ictp_step1

!---

## MOOSE Input

An input file defines a MOOSE simulation and follows a standardized syntax, the "hierarchical input text" (HIT) format.

A basic MOOSE input file *typically* has six "blocks":

- `[Mesh]`: Define the geometry of the domain
- `[Variables]`: Define the unknown(s) of the problem
- `[Kernels]`: Define the equation(s) to solve
- `[BCs]`: Define the boundary condition(s) of the problem
- `[Executioner]`: Define how the problem will be solved
- `[Outputs]`: Define how the solution will be returned

There are caveats to the above and many other blocks exist. Shortcut syntax can also be created to simplify input.

A listing of all syntax can be found [here](syntax/index.md).

The [Moose Language Support](https://marketplace.visualstudio.com/items?itemName=DanielSchwen.moose-language-support) VSCode extension adds diagnostics and autocompletion to input in VSCode.

!---

## Introduction to Input

We will work through the process of mesh generation as an introduction to building MOOSE input.

The first step will be the generation of a mesh that is uniform two-dimensional grid using the [`GeneratedMeshGenerator`](GeneratedMeshGenerator.md).

!---

## Simple Mesh Input

!listing ictp/inputs/step1-1_meshing_basic.i

The above defines a [`GeneratedMeshGenerator`](GeneratedMeshGenerator.md) named `gmg` with the parameters:

- `dim`: sets the dimension to $2$
- `nx`: which builds $10$ elements in the $x$-direction
- `ny`: which builds $10$ elements in the $y$-direction

!---

## Run: Simple Mesh Input

```bash
$ cardinal-opt -i step1-1_meshing.basic.i --mesh-only
```

- The argument `-i` specifies which input file to run
- The `--mesh-only` argument forces only mesh generation and output in Exodus format

  - Without arguments to `--mesh-only`, the mesh will be output as Exodus in the same folder as the input with the name `<input_file>_out.e`, i.e., `step1-1_meshing_basic_out.e`
  - If an argument is provided to `--mesh-only`, that argument will be used as the output path for the generated mesh

- This output format, Exodus, is commonly inspected using Paraview

!---

## Result: Simple Mesh Input

!style halign=center
!media step1-1_mesh.png style=width:40%

!---

## MeshGenerator System

The previous example utilized the MOOSE [`MeshGenerator`](Mesh/index.md) system:

- Enables the programmatic construction of a mesh
- Generation can be chained together through dependencies so that complex meshes may be built up from a series of simple processes
- Several built-in generators exist but you can develop your own MeshGenerators

  - Built-in examples: building grids and concentric circles; triangulation and tetrahedralization; extrusion; renaming and merging of blocks and sidesets; stitching; scaling, translating, and rotating; image conversion
- Supports distributed mesh generation and modification
- Enables the addition of "metadata" to meshes

  - Particularly useful in neutronics for storing things like depletion zones

All of our examples that follow (including the Cardinal tutorial) will utilize the [`MeshGenerator`](Mesh/index.md) system. Of significant note is the [`Reactor`](reactor/index.md) module, which supports the systematic mesh generation of common reactor types.

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

## Other Input Syntax

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

The grid we first generated isn't particularly interesting.

Let's generate a mesh that represents represents a fuel pin surrounded by water in a square lattice using the [`ConcentricCircleMeshGenerator`](ConcentricCircleMeshGenerator.md) with the following dimensions:


| Quantity | Value |
| -: | :- |
| Hole radius | $0.08$ cm |
| Pellet radius | $0.26$ cm |
| Clad radius | $0.3$ cm |
| Pitch | $0.695$ cm |

We will also apply a special treatment at the clad-fluid interface to support a thermal-hydraulics solve with a boundary layer.

!--

## Input: Concentric Circle Mesh

!listing ictp/inputs/step1-2_concentric_circle.i

!--

## Run: Concentric Circle Mesh

```bash
$ cardinal-opt -i step1-2_concentric_circle.i --mesh-only
```

```
 Mesh Information:
  elem_dimensions()={2}
  elem_default_orders()={FIRST}
  supported_nodal_order()=1
  spatial_dimension()=2
  n_nodes()=729
    n_local_nodes()=729
  n_elem()=688
    n_local_elem()=688
    n_active_elem()=688
  n_subdomains()=5
  n_elemsets()=0
  n_partitions()=1
  n_processors()=1
  n_threads()=1
  processor_id()=0
  is_prepared()=true
  is_replicated()=true

 Mesh Bounding Box:
  Minimum: (x,y,z)=(-0.003475, -0.003475,        0)
  Maximum: (x,y,z)=(0.003475, 0.003475,        0)
  Delta:   (x,y,z)=( 0.00695,  0.00695,        0)

 Mesh Element Type(s):
  QUAD4

 Mesh Nodesets:
  Nodeset 1, 21 nodes
   Bounding box minimum: (x,y,z)=(-0.003475, -0.003475,        0)
   Bounding box maximum: (x,y,z)=(-0.003475, 0.003475,        0)
   Bounding box delta: (x,y,z)=(4.33681e-19,  0.00695,        0)
  Nodeset 2, 21 nodes
   Bounding box minimum: (x,y,z)=(-0.003475, -0.003475,        0)
   Bounding box maximum: (x,y,z)=(0.003475, -0.003475,        0)
   Bounding box delta: (x,y,z)=( 0.00695, 8.67362e-19,        0)
  Nodeset 3, 21 nodes
   Bounding box minimum: (x,y,z)=(0.003475, -0.003475,        0)
   Bounding box maximum: (x,y,z)=(0.003475, 0.003475,        0)
   Bounding box delta: (x,y,z)=(4.33681e-19,  0.00695,        0)
  Nodeset 4, 21 nodes
   Bounding box minimum: (x,y,z)=(-0.003475, 0.003475,        0)
   Bounding box maximum: (x,y,z)=(0.003475, 0.003475,        0)
   Bounding box delta: (x,y,z)=( 0.00695,        0,        0)

 Mesh Sidesets:
  Sideset 1 (left), 20 sides (EDGE2), 20 elems (QUAD4), 21 nodes
   Side volume: 0.00695
   Bounding box minimum: (x,y,z)=(-0.003475, -0.003475,        0)
   Bounding box maximum: (x,y,z)=(-0.003475, 0.003475,        0)
   Bounding box delta: (x,y,z)=(4.33681e-19,  0.00695,        0)
  Sideset 2 (bottom), 20 sides (EDGE2), 20 elems (QUAD4), 21 nodes
   Side volume: 0.00695
   Bounding box minimum: (x,y,z)=(-0.003475, -0.003475,        0)
   Bounding box maximum: (x,y,z)=(0.003475, -0.003475,        0)
   Bounding box delta: (x,y,z)=( 0.00695, 8.67362e-19,        0)
  Sideset 3 (right), 20 sides (EDGE2), 20 elems (QUAD4), 21 nodes
   Side volume: 0.00695
   Bounding box minimum: (x,y,z)=(0.003475, -0.003475,        0)
   Bounding box maximum: (x,y,z)=(0.003475, 0.003475,        0)
   Bounding box delta: (x,y,z)=(4.33681e-19,  0.00695,        0)
  Sideset 4 (top), 20 sides (EDGE2), 20 elems (QUAD4), 21 nodes
   Side volume: 0.00695
   Bounding box minimum: (x,y,z)=(-0.003475, 0.003475,        0)
   Bounding box maximum: (x,y,z)=(0.003475, 0.003475,        0)
   Bounding box delta: (x,y,z)=( 0.00695,        0,        0)

 Mesh Edgesets:
  None

 Mesh Subdomains:
  Subdomain 1: 192 elems (QUAD4, 192 active), 217 active nodes
   Volume: 2.00488e-06
   Bounding box minimum: (x,y,z)=( -0.0008,  -0.0008,        0)
   Bounding box maximum: (x,y,z)=(  0.0008,   0.0008,        0)
   Bounding box delta: (x,y,z)=(  0.0016,   0.0016,        0)
  Subdomain 2: 144 elems (QUAD4, 144 active), 192 active nodes
   Volume: 1.91717e-05
   Bounding box minimum: (x,y,z)=( -0.0026,  -0.0026,        0)
   Bounding box maximum: (x,y,z)=(  0.0026,   0.0026,        0)
   Bounding box delta: (x,y,z)=(  0.0052,   0.0052,        0)
  Subdomain 3: 48 elems (QUAD4, 48 active), 96 active nodes
   Volume: 7.01709e-06
   Bounding box minimum: (x,y,z)=(  -0.003,   -0.003,        0)
   Bounding box maximum: (x,y,z)=(   0.003,    0.003,        0)
   Bounding box delta: (x,y,z)=(   0.006,    0.006,        0)
  Subdomain 4: 48 elems (QUAD4, 48 active), 96 active nodes
   Volume: 8.99867e-07
   Bounding box minimum: (x,y,z)=(-0.0030475, -0.0030475,        0)
   Bounding box maximum: (x,y,z)=(0.0030475, 0.0030475,        0)
   Bounding box delta: (x,y,z)=(0.006095, 0.006095,        0)
  Subdomain 5: 256 elems (QUAD4, 256 active), 320 active nodes
   Volume: 1.9209e-05
   Bounding box minimum: (x,y,z)=(-0.003475, -0.003475,        0)
   Bounding box maximum: (x,y,z)=(0.003475, 0.003475,        0)
   Bounding box delta: (x,y,z)=( 0.00695,  0.00695,        0)
  Global mesh volume = 4.83025e-05
```

!---

## Result: Concentric Circle Mesh Input

!style halign=center
!media step1-2_mesh.png style=width:25%

This mesh has five blocks (also called subdomains):

- $1$: Inner hole (grey)
- $2$: Fuel (red)
- $3$: Cladding (green)
- $4$: Fluid boundary layer (blue)
- $5$: Remaining fluid (yellow)

!---

## Separating Meshes

Later in this tutorial, we will be treating the fuel pin and the surrounding fluid independently. Thus, we will now take the mesh that we just generated and separate it into two components: one for the fuel pin and one for the fluid.

This task reveals a capability of MeshGenerators -- they can be combined together. Some MeshGenerators generate meshes, some modify meshes, and some do both. Oftentimes a generator will have an input parameter called `input`, which is used as the name of another MeshGenerator to be used as input to modify.

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

!listing ictp/inputs/step1-3_fuel_pin.i

!--

## Run: Fuel Mesh

Here, we add a value to `--mesh-only` that stores the resulting mesh in `fuel_pin.e`:

```bash
$ cardinal-opt -i step1-3_fuel_pin.i --mesh-only fuel_pin.e
```

```
 Mesh Information:
  elem_dimensions()={2}
  elem_default_orders()={FIRST}
  supported_nodal_order()=1
  spatial_dimension()=2
  n_nodes()=240
    n_local_nodes()=240
  n_elem()=192
    n_local_elem()=192
    n_active_elem()=192
  n_subdomains()=2
  n_elemsets()=0
  n_partitions()=1
  n_processors()=1
  n_threads()=1
  processor_id()=0
  is_prepared()=true
  is_replicated()=true

 Mesh Bounding Box:
  Minimum: (x,y,z)=(  -0.003,   -0.003,        0)
  Maximum: (x,y,z)=(   0.003,    0.003,        0)
  Delta:   (x,y,z)=(   0.006,    0.006,        0)

 Mesh Element Type(s):
  QUAD4

 Mesh Nodesets:
  Nodeset 5, 48 nodes
   Bounding box minimum: (x,y,z)=( -0.0008,  -0.0008,        0)
   Bounding box maximum: (x,y,z)=(  0.0008,   0.0008,        0)
   Bounding box delta: (x,y,z)=(  0.0016,   0.0016,        0)
  Nodeset 6, 48 nodes
   Bounding box minimum: (x,y,z)=(  -0.003,   -0.003,        0)
   Bounding box maximum: (x,y,z)=(   0.003,    0.003,        0)
   Bounding box delta: (x,y,z)=(   0.006,    0.006,        0)

 Mesh Sidesets:
  Sideset 5 (inner), 48 sides (EDGE2), 48 elems (QUAD4), 48 nodes
   Side volume: 0.00502296
   Bounding box minimum: (x,y,z)=( -0.0008,  -0.0008,        0)
   Bounding box maximum: (x,y,z)=(  0.0008,   0.0008,        0)
   Bounding box delta: (x,y,z)=(  0.0016,   0.0016,        0)
  Sideset 6 (outer), 48 sides (EDGE2), 48 elems (QUAD4), 48 nodes
   Side volume: 0.0188361
   Bounding box minimum: (x,y,z)=(  -0.003,   -0.003,        0)
   Bounding box maximum: (x,y,z)=(   0.003,    0.003,        0)
   Bounding box delta: (x,y,z)=(   0.006,    0.006,        0)

 Mesh Edgesets:
  None

 Mesh Subdomains:
  Subdomain 2 (fuel): 144 elems (QUAD4, 144 active), 192 active nodes
   Volume: 1.91717e-05
   Bounding box minimum: (x,y,z)=( -0.0026,  -0.0026,        0)
   Bounding box maximum: (x,y,z)=(  0.0026,   0.0026,        0)
   Bounding box delta: (x,y,z)=(  0.0052,   0.0052,        0)
  Subdomain 3 (clad): 48 elems (QUAD4, 48 active), 96 active nodes
   Volume: 7.01709e-06
   Bounding box minimum: (x,y,z)=(  -0.003,   -0.003,        0)
   Bounding box maximum: (x,y,z)=(   0.003,    0.003,        0)
   Bounding box delta: (x,y,z)=(   0.006,    0.006,        0)
  Global mesh volume = 2.61888e-05
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

!listing ictp/inputs/step1-4_fluid.i

!---

## Run: Fluid Mesh

Generate the mesh and output it to `fluid.e`:

```bash
$ cardinal-opt -i step1-4_fluid.i --mesh-only fluid.e
```

```
 Mesh Information:
  elem_dimensions()={2}
  elem_default_orders()={FIRST}
  supported_nodal_order()=1
  spatial_dimension()=2
  n_nodes()=368
    n_local_nodes()=368
  n_elem()=304
    n_local_elem()=304
    n_active_elem()=304
  n_subdomains()=1
  n_elemsets()=0
  n_partitions()=1
  n_processors()=1
  n_threads()=1
  processor_id()=0
  is_prepared()=true
  is_replicated()=true

 Mesh Bounding Box:
  Minimum: (x,y,z)=(-0.003475, -0.003475,        0)
  Maximum: (x,y,z)=(0.003475, 0.003475,        0)
  Delta:   (x,y,z)=( 0.00695,  0.00695,        0)

 Mesh Element Type(s):
  QUAD4

 Mesh Nodesets:
  Nodeset 4, 80 nodes
   Bounding box minimum: (x,y,z)=(-0.003475, -0.003475,        0)
   Bounding box maximum: (x,y,z)=(0.003475, 0.003475,        0)
   Bounding box delta: (x,y,z)=( 0.00695,  0.00695,        0)
  Nodeset 5, 48 nodes
   Bounding box minimum: (x,y,z)=(  -0.003,   -0.003,        0)
   Bounding box maximum: (x,y,z)=(   0.003,    0.003,        0)
   Bounding box delta: (x,y,z)=(   0.006,    0.006,        0)

 Mesh Sidesets:
  Sideset 4 (outer), 80 sides (EDGE2), 76 elems (QUAD4), 80 nodes
   Side volume: 0.0278
   Bounding box minimum: (x,y,z)=(-0.003475, -0.003475,        0)
   Bounding box maximum: (x,y,z)=(0.003475, 0.003475,        0)
   Bounding box delta: (x,y,z)=( 0.00695,  0.00695,        0)
  Sideset 5 (inner), 48 sides (EDGE2), 48 elems (QUAD4), 48 nodes
   Side volume: 0.0188361
   Bounding box minimum: (x,y,z)=(  -0.003,   -0.003,        0)
   Bounding box maximum: (x,y,z)=(   0.003,    0.003,        0)
   Bounding box delta: (x,y,z)=(   0.006,    0.006,        0)

 Mesh Edgesets:
  None

 Mesh Subdomains:
  Subdomain 4 (water): 304 elems (QUAD4, 304 active), 368 active nodes
   Volume: 2.01088e-05
   Bounding box minimum: (x,y,z)=(-0.003475, -0.003475,        0)
   Bounding box maximum: (x,y,z)=(0.003475, 0.003475,        0)
   Bounding box delta: (x,y,z)=( 0.00695,  0.00695,        0)
  Global mesh volume = 2.01088e-05
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
