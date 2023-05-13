# Example: Heat Pipe-Cooled Micro Reactor (HP-MR)

!---

## Heat Pipe-Cooled Micro Reactor (HP-MR)

!row!
!col small=12 medium=6 large=8

- This section covers the creation of a detailed 1/6 core Heat-Pipe Micro Reactor mesh using the following key steps:

  - Create fuel/moderator/heat-pipe pin cells
  - Combine pins into a fuel assembly
  - Create control drum assembly
  - Create additional assemblies (reflector, central hole, dummies)
  - Combine assemblies into a full core
  - Delete dummy assemblies
  - Add a core periphery region
  - Slice full core to 1/6 core
  - Extrude 2D mesh to 3D

!col small=12 medium=6 large=4

!media tutorial04_meshing/hpmr_workflow.png
       style=width:100%;display:block;margin-left:auto;margin-right:auto;

!row-end!

+Hands-on package MOOSE input file+: `combined/reactor_workshop/tests/reactor_examples/hpmr/hpmr.i`

!---

## Create Pin Unit Cell

- [PolygonConcentricCircleMeshGenerator.md]

- Center, outer ring, background region
- Volumes preserved ([!param](/Mesh/PolygonConcentricCircleMeshGenerator/preserve_volumes)=`True`)
- TRI center elements ([!param](/Mesh/PolygonConcentricCircleMeshGenerator/quad_center_elements)=`False`)

- Center of pin unit cell can be meshed by triangular or quadratic elements
- Duct regions can also be added to the outer periphery (not used in this example)
- Block IDs are assigned for each radial layer

!row!
!col small=12 medium=6 large=8

!listing reactor_examples/hpmr/hpmr.i
         block=Mesh/moderator_pincell
         link=False

!col small=12 medium=6 large=4

!media tutorial04_meshing/hpmr_pin_cell.png
       style=width:100%;display:block;margin-left:auto;margin-right:auto;

!row-end!

!---

## Create Patterned Hexagonal Fuel Assembly

- [PatternedHexMeshGenerator.md]

- Hexagonal grid (13.376 units apothem )
- 3 input pin types (fuel, heatpipe, moderator)
- Background region out to hexagonal boundary
- Pattern references input mesh list in order (0-indexed)

!row!
!col small=12 medium=6 large=8

!listing reactor_examples/hpmr/hpmr.i
         block=Mesh/fuel_assembly
         link=False

!col small=12 medium=6 large=4

!media tutorial04_meshing/hpmr_hex_assembly.png
       style=width:100%;display:block;margin-left:auto;margin-right:auto;

!row-end!

!---

## Fuel-Only Core

- [PatternedHexMeshGenerator.md]

- 61 assembly core
- Whole core rotated 60 degrees

!row!
!col small=12 medium=6 large=8

!listing reactor_examples/hpmr/hpmr.i
         block=Mesh/fuel_core
         link=False

!col small=12 medium=6 large=4

!media tutorial04_meshing/hpmr_fuel_core.png
       style=width:100%;display:block;margin-left:auto;margin-right:auto;

!row-end!

!---

## Control Drum Assembly

1. Define control drum mesh with [HexagonConcentricCircleAdaptiveBoundaryMeshGenerator.md]
2. Split outer ring into 2 separate block IDs using [AzimuthalBlockSplitGenerator.md] to account for control material zone

Setup

- [HexagonConcentricCircleAdaptiveBoundaryMeshGenerator.md]
- [AzimuthalBlockSplitGenerator.md]

- 3 regions (Center region, Absorber Ring, Hexagonal background)

- Circular volumes preserved regardless of meshing fidelity ([!param](/Mesh/PolygonConcentricCircleMeshGenerator/preserve_volumes)=`true`)
- Side node adaptation performed for instances with neighboring fuel assemblies
- [HexagonConcentricCircleAdaptiveBoundaryMeshGenerator.md] is used in this case to create a "single pin" hexagonal assembly with sides matching another mesh

!---

### Control Drum Assembly

!row!
!col small=12 medium=6 large=8

!listing reactor_examples/hpmr/hpmr.i
         block=Mesh/cd1_step1
         link=False

!col small=12 medium=6 large=4

!media tutorial04_meshing/hpmr_control_drum_1.png
       style=width:1000%;display:block;margin-left:auto;margin-right:auto;

!row-end!

!row!
!col small=12 medium=6 large=8

!listing reactor_examples/hpmr/hpmr.i
         block=Mesh/cd1
         link=False

!col small=12 medium=6 large=4

!media tutorial04_meshing/hpmr_control_drum_2.png
       style=width:100%;display:block;margin-left:auto;margin-right:auto;

!row-end!

!---

## Create Additional Assemblies (Reflector, Air, Dummy)

- [HexagonConcentricCircleAdaptiveBoundaryMeshGenerator.md]

- Same size as fuel assemblies
- Side node adaptation for case with neighboring fuel assemblies

- Single central air hole assembly
- Multiple "Dummy" assemblies needed for patterning (to be deleted later)

- Boundary nodes on two neighboring assemblies need to match up in order for the assemblies to be stitched together
- Assemblies that neighbor several different assembly types (control drums, reflectors) are generally created last so that the boundaries can be specified based on the meshes these need to match.

!row!
!col small=12 medium=6 large=8

!listing reactor_examples/hpmr/hpmr.i
         block=Mesh/refl1
         link=False

!col small=12 medium=6 large=4

!media tutorial04_meshing/hpmr_additional_assemblies.png
       style=width:100%;display:block;margin-left:auto;margin-right:auto;

!row-end!

!---

## Create Patterned Full Core

- [PatternedHexMeshGenerator.md]

- Whole core rotated 60 degrees
- 5 Input geometry types

  - Fuel assemblies
  - "Other" Assemblies - Control drum (12 meshes), Reflector (6 meshes), Air hole center
  - Dummy assemblies

- "Empty" spots should be defined with dummy assemblies and later deleted.

!row!
!col small=12 medium=6 large=8

!listing reactor_examples/hpmr/hpmr.i
         block=Mesh/core
         link=False

!col small=12 medium=6 large=4

!media tutorial04_meshing/hpmr_core.png
       style=width:100%;display:block;margin-left:auto;margin-right:auto;

!row-end!

!---

## Delete Dummy Assemblies

!row!
!col small=12 medium=6 large=8

- [BlockDeletionGenerator.md]

- Remove "dummy" assemblies which were added only for core hex patterning

- The blocks IDs of the dummy assembly were defined by the user in the definition of the dummy assembly
- Set the new outer boundaries that result from the deleted assemblies to have the same sideset ID as the existing outer boundary

!col small=12 medium=6 large=4

!media tutorial04_meshing/hpmr_del_dummy.png
       style=width:100%;display:block;margin-left:auto;margin-right:auto;

!row-end!

!listing reactor_examples/hpmr/hpmr.i
         block=Mesh/del_dummy
         link=False

!---

## Add Core Periphery

- [PeripheralRingMeshGenerator.md]

- 115.0 units vessel radius

- Use existing core outer boundary ID as input boundary which describes which boundary the peripheral ring should start from

!row!
!col small=12 medium=6 large=8

!listing reactor_examples/hpmr/hpmr.i
         block=Mesh/outer_shield
         link=False

!col small=12 medium=6 large=4

!media tutorial04_meshing/hpmr_periphery.png
       style=width:100%;display:block;margin-left:auto;margin-right:auto;

!row-end!

!---

## Slice to 1/6 Core

!row!
!col small=12 medium=6 large=8

- [PlaneDeletionGenerator.md]

- Trimming plane defined by a point and a normal vector
- Elements whose centroids lie "above" (in the direction of the normal vector) the plane will be deleted
- Set new outer boundary ID on sliced lines to be referenced later
- To slice the full core in half:

  - +Point+ = $(0, 0, 0)$, +normal+ = $(10, 17.32, 0)$

- To slice the half core into 1/3:

  - +Point+ = $(0, 0, 0)$, +normal+ = $(10, -17.32, 0)$

- Advanced trimming options are available (see [HexagonMeshTrimmer.md]). Trimming should only be performed along lines of symmetry.

!col small=12 medium=6 large=4

!media tutorial04_meshing/hpmr_slice.png
       style=width:100%;display:block;margin-left:auto;margin-right:auto;

!row-end!

!row!
!col small=12 medium=6 large=8

!listing reactor_examples/hpmr/hpmr.i
         block=Mesh/coreslice_1
         link=False

!col small=12 medium=6 large=4

!listing reactor_examples/hpmr/hpmr.i
         block=Mesh/coreslice_2
         link=False

!row-end!

!---

## Extrude to 3D

!row!
!col small=12 medium=6 large=8

- [AdvancedExtruderGenerator.md]

- Extrude the 2D $(x,y)$ mesh in +$z$ direction $(0, 0, 1)$

- Split into 3 axial intervals
- Heights for each interval: 20 units, 160 units, 20 units

  - Number of layers in each interval: 1, 8, 1

- Set top/bottom boundary IDs to be referenced later

!col small=12 medium=6 large=4

!media tutorial04_meshing/hpmr_extrude.png
       style=width:75%;display:block;margin-left:auto;margin-right:auto;

!row-end!

!listing reactor_examples/hpmr/hpmr.i
         block=Mesh/extrude
         link=False

!---

## Using the Mesh in Downstream Physics Applications (Griffin)

We briefly describe some key steps which are required to use the resulting mesh in Griffin. Namely, block IDs (`subdomain_id`) are referenced in Griffin to assign materials to elements. The external boundary sideset IDs are referenced in Griffin to assign boundary conditions.

!---

### Assignment of Material Properties to Blocks

All blocks in the mesh must be assigned to a material in Griffin.

!row!
!col small=12 medium=6 large=8

!listing reactor_examples/hpmr/hpmr_griffin_snippet.i
         block=Materials
         link=False

!col small=12 medium=6 large=4

!media tutorial04_meshing/hpmr_blocks.png
       style=width:100%;display:block;margin-left:auto;margin-right:auto;

!row-end!

!---

### Assignment of Boundary Conditions to Sidesets

!media tutorial04_meshing/hpmr_sidesets.png
       style=width:100%;display:block;margin-left:auto;margin-right:auto;

!---

### Generation of Coarse Mesh in Griffin

We briefly touch on mesh generation for the Coarse Mesh Finite Difference acceleration option in Griffin. There are three options:

- Option 1: Define "coarse" mesh that is identical to fine mesh
- Option 2: Define coarse mesh covering the same geometry as the fine mesh but with a coarser mesh refinement
- Option 3: Define a regular square grid covering the entire mesh domain (and beyond)

!row!
!col small=12 medium=6 large=8

!media tutorial04_meshing/hpmr_coarse_mesh_cartesian_stencil_2.png
       style=width:100%;display:block;margin-left:auto;margin-right:auto;

!col small=12 medium=6 large=4

!media tutorial04_meshing/hpmr_coarse_mesh_cartesian_stencil_1.png
       style=width:100%;display:block;margin-left:auto;margin-right:auto;

!row-end!
