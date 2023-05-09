# Example: Heat Pipe-Cooled Micro Reactor (HP-MR)

This section covers the creation of a detailed 1/6 core [Heat-Pipe Micro Reactor](https://mooseframework.inl.gov/virtual_test_bed/microreactors/mrad/mrad_model.html) mesh using the following key steps:

1. Create fuel/moderator/heat-pipe pin cells
2. Combine pins into a fuel assembly
3. Create control drum assembly
4. Create additional assemblies (reflector, central hole, dummies)
5. Combine assemblies into a full core
6. Delete dummy assemblies
7. Add a core periphery region
8. Slice full core to 1/6 core
9. Extrude 2D mesh to 3D

!media tutorial04_meshing/hpmr_workflow.png
       id=tutorial04-hpmr_workflow
       caption=Full workflow of HP-MR example mesh.
       style=width:75%;display:block;margin-left:auto;margin-right:auto;

## Create Pin Unit Cell

The pin cell represents the most basic element in the hierarchical structure of a reactor core. In the case of a heat-pipe micro reactor, typical pin cells include fuel, heat pipe, and moderator. These components are all generated using the [PolygonConcentricCircleMeshGenerator.md]. Every pin cell mesh property (e.g., ring radii and intervals) can be tailored to specific needs. The following example shows construction of the moderator pin cell. The heat pipe and fuel pin cells are constructed similarly.

### Object

- [PolygonConcentricCircleMeshGenerator.md]

### Geometry Features

- Center region

  - 0.825 unit radius
  - 2 intervals thick

- Outer Ring

  - 0.92 unit radius
  - 1 interval

- Hexagonal background

  - 1.15 units apothem
  - 1 interval

- Volumes preserved ([!param](/Mesh/PolygonConcentricCircleMeshGenerator/preserve_volumes)=`True`)
- TRI center elements ([!param](/Mesh/PolygonConcentricCircleMeshGenerator/quad_center_elements)=`False`)

### Notes

- Center of pin unit cell can be meshed by triangular or quadratic elements
- Duct regions can also be added to the outer periphery (not used in this example)
- Block IDs are assigned for each radial layer

### Example

!media tutorial04_meshing/hpmr_pin_cell.png
       id=tutorial04-hpmr_pin_cell
       caption=Moderator Pin Cell.
       style=width:50%;display:block;margin-left:auto;margin-right:auto;

!listing reactor_examples/hpmr/hpmr.i
         id=tutorial04-HPMR_9.1_Create_pin_unit_cell
         caption=Moderator Pin Cell.
         block=Mesh/moderator_pincell

## Create Patterned Hexagonal Fuel Assembly

Generated pin cell meshes (fuel, heat pipe, and moderator) are stitched into a fuel assembly using [PatternedHexMeshGenerator.md].

### Object

- [PatternedHexMeshGenerator.md]

### Geometry Features

- Hexagonal grid

  - 13.376 units apothem
  - Note: requires hexagonal input pins

- 3 input pin types

  - fuel_pincell
  - heatpipe_pincell
  - moderator_pincell (detailed in pervious section)

- Background region out to hexagonal boundary

  - 1 interval

### Notes

- Pattern references input mesh list in order (0-indexed)
- Duct regions can also be added to the outer periphery (not used in this example)

### Example

!media tutorial04_meshing/hpmr_hex_assembly.png
       id=tutorial04-hpmr_hex_assembly
       caption=Fuel hex assembly.
       style=width:50%;display:block;margin-left:auto;margin-right:auto;

!listing reactor_examples/hpmr/hpmr.i
         id=tutorial04-HPMR_9.2_Patterned_Hex_Assembly
         caption=Fuel hex assembly.
         block=Mesh/fuel_assembly

## Fuel-Only Core

The HP-MR utilizes several different assembly types, but as simplified example, consider a core composed only of fuel assemblies.

### Object

- [PatternedHexMeshGenerator.md]

### Geometry Features

- 61 assembly core
- Whole core rotated 60&deg;

### Notes

### Example

!media tutorial04_meshing/hpmr_fuel_core.png
       id=tutorial04-hpmr_fuel_core
       caption=Full fuel core assembly.
       style=width:50%;display:block;margin-left:auto;margin-right:auto;

!listing reactor_examples/hpmr/hpmr.i
         id=tutorial04-HPMR_9.3_Patterned_Hex_Core
         caption=Full fuel core assembly.
         block=Mesh/fuel_core

## Control Drum Assembly

Control drums are a type of control device used in some small reactor systems to regulate the reactor's power output and maintain a stable operating condition. The generation of control drum mesh involves two steps:

1. Define control drum mesh with [HexagonConcentricCircleAdaptiveBoundaryMeshGenerator.md]
2. Split outer ring into 2 separate block IDs using [AzimuthalBlockSplitGenerator.md] to account for control material zone.

### Object

- [HexagonConcentricCircleAdaptiveBoundaryMeshGenerator.md]
- [AzimuthalBlockSplitGenerator.md]

### Geometry Features

- Center region

  - 12.25 unit radius
  - 2 radial intervals

- Absorber Ring

  - 13.25 unit radius
  - 1 radial interval
  - Outer ring block ID from 45&deg; to 135&deg; is changed to a different ID for this case using [AzimuthalBlockSplitGenerator.md] in a second step

- Hexagonal background

  - 13.376 unit apothem (half-pitch)
  - 4 azimuthal sectors on each hexagon face, except for the two faces which require more azimuthal sectors to match neighboring fuel assemblies
  - 2 radial intervals

### Notes

- Circular volumes preserved regardless of meshing fidelity ([!param](/Mesh/PolygonConcentricCircleMeshGenerator/preserve_volumes)=`true`)
- Side node adaptation performed for instances with neighboring fuel assemblies. The reference side number (sides_to_adapt) and assembly neighbor meshes (inputs) are taken as input so that the nodes can be placed correctly on the control drum mesh to match the neighbor meshes.
- [HexagonConcentricCircleAdaptiveBoundaryMeshGenerator.md] is used in this case to create a "single pin" hexagonal assembly with sides matching another mesh. For example, in cases where the node counts on the edges of the control drum and fuel assembly differ, the [HexagonConcentricCircleAdaptiveBoundaryMeshGenerator.md] can automatically adjust the mesh density along the control drum's boundary to ensure compatibility.
- This meshing method is intended for analyzing a static control drum position. For transient control drum rotation (and control rod insertion), this azimuthal block split is not needed as Griffin uses special material definitions to define the absorber location as a function of time.

### Example (Step 1 - Generate Control Drum)

!media tutorial04_meshing/hpmr_control_drum_1.png
       id=tutorial04-hpmr_control_drum_1
       caption=Control Drum Generation.
       style=width:50%;display:block;margin-left:auto;margin-right:auto;

!listing reactor_examples/hpmr/hpmr.i
         id=tutorial04-HPMR_9.4_Control_Drum_Assembly-1
         caption=Control Drum Generation.
         block=Mesh/cd1_step1

### Example (Step 2 - Block Splitting to Define Absorber Segment)

!media tutorial04_meshing/hpmr_control_drum_2.png
       id=tutorial04-hpmr_control_drum_2
       caption=Block ID swaps.
       style=width:50%;display:block;margin-left:auto;margin-right:auto;

!listing reactor_examples/hpmr/hpmr.i
         id=tutorial04-HPMR_9.4_Control_Drum_Assembly-2
         caption=Block ID swaps.
         block=Mesh/cd1

## Create Additional Assemblies (Reflector, Air, Dummy)

Other components of the reactor, such as reflectors and air holes, can also be created using the [HexagonConcentricCircleAdaptiveBoundaryMeshGenerator.md]. Additionally, to achieve the necessary hexagonal shape for the [PatternedHexMeshGenerator.md], dummy blocks are required. These dummy blocks will be eliminated once the reactor core is constructed.

### Object

- [HexagonConcentricCircleAdaptiveBoundaryMeshGenerator.md]

### Geometry Features

- 13.376 unit apothem

  - 4 azimuthal sectors on each hexagon side, except where more required for conformal meshing with neighbors (two sides of reflector, all sides of air hole)
  - 2 radial intervals
  - No concentric circles included (no pins)

- Side node adaptation for case with neighboring fuel assemblies

  - Reflector assemblies along the core outer boundary (multiple reflector geometries needed based on location in the core)

- Single central air hole assembly
- Multiple "Dummy" assemblies needed for patterning (to be deleted later), all with identical boundary densities (only one geometry needed, with same block ID for easy deletion later).

### Notes

- The boundary nodes on two neighboring assemblies need to match up in order for the assemblies to be stitched together. Assemblies that neighbor several different assembly types (control drums, reflectors) are generally created last so that the boundaries can be specified based on the meshes these need to match.

### Example

!media tutorial04_meshing/hpmr_additional_assemblies.png
       id=tutorial04-hpmr_additional_assemblies
       caption=Additional assembly types.
       style=width:50%;display:block;margin-left:auto;margin-right:auto;

!listing reactor_examples/hpmr/hpmr.i
         id=tutorial04-HPMR_9.5_Additional_Assemblies_Reflector
         caption=Reflector assembly example.
         block=Mesh/refl1

!listing reactor_examples/hpmr/hpmr.i
         id=tutorial04-HPMR_9.5_Additional_Assemblies_Air
         caption=Air center assembly example.
         block=Mesh/air_center

!listing reactor_examples/hpmr/hpmr.i
         id=tutorial04-HPMR_9.5_Additional_Assemblies_Dummy
         caption=Dummy assembly example.
         block=Mesh/dummy

## Create Patterned Full Core

Generated reactor component meshes (fuel assembly, control drum, reflector, air hole and dummy) are stitched into a 2D hexagonal lattice (reactor core) using [PatternedHexMeshGenerator.md].

### Object

- [PatternedHexMeshGenerator.md]

### Geometry Features

- Outer hex boundary disabled ([!param](/Mesh/PatternedHexMeshGenerator/pattern_boundary)=`none`)
- Whole core rotated 60&deg;
- 5 Input geometry types

  - Fuel assembly
  - Control drum (12 meshes created for different boundary arrangements)
  - Reflector (6 meshes created for different boundary arrangements)
  - Air hole center
  - Dummy assemblies

### Notes

- [PatternedHexMeshGenerator.md] requires a "perfect" hex pattern. "Empty" spots should be defined with dummy assemblies and later deleted.

### Example

!media tutorial04_meshing/hpmr_core.png
       id=tutorial04-hpmr_core
       caption=Full core assembly.
       style=width:50%;display:block;margin-left:auto;margin-right:auto;

!listing reactor_examples/hpmr/hpmr.i
         id=tutorial04-HPMR_9.6_Full_Core
         caption=Full core assembly.
         block=Mesh/core

## Delete Dummy Assemblies

Dummy blocks were used in the previous step to facilitate the core pattern but are not part of the final mesh. We now delete them.

### Object

- [BlockDeletionGenerator.md]

### Geometry Features

- Remove "dummy" assemblies which were added only for core hex patterning

### Notes

- The blocks IDs of the dummy assembly were defined by the user in the definition of the dummy assembly
- Set the new outer boundaries that result from the deleted assemblies to have the same sideset ID as the existing outer boundary (this step is crucial to ensure the entire outer boundary can be referenced in Griffin for sideset assignment)

### Example

!media tutorial04_meshing/hpmr_del_dummy.png
       id=tutorial04-hpmr_del_dummy
       caption=Full core assembly without dummies.
       style=width:50%;display:block;margin-left:auto;margin-right:auto;

!listing reactor_examples/hpmr/hpmr.i
         id=tutorial04-HPMR_9.7_Delete_Dummies
         caption=Full core assembly without dummies.
         block=Mesh/del_dummy

## Add Core Periphery

A peripheral ring (shield) can be incorporated into the 2D reactor core using either the [PeripheralRingMeshGenerator.md] or the [PeripheralTriangleMeshGenerator.md]. The former generates a quad mesh, while the latter generates a tri mesh. These two mesh generators also allow the user to control mesh density. Using the quadrilateral mesh option permits easier symmetry trimming options down the line.

### Object

- [PeripheralRingMeshGenerator.md]

### Geometry Features

- 115.0 units vessel radius

  - 1 radial interval

### Notes

- Use existing core outer boundary ID as input boundary which describes which boundary the peripheral ring should start from

### Example

!media tutorial04_meshing/hpmr_periphery.png
       id=tutorial04-hpmr_periphery
       caption=Core periphery.
       style=width:50%;display:block;margin-left:auto;margin-right:auto;

!listing reactor_examples/hpmr/hpmr.i
         id=tutorial04-HPMR_9.8_Add_Core_Periphery
         caption=Core periphery.
         block=Mesh/outer_shield

## Slice to 1/6 Core

To conserve computational resources, users may create a half core or 1/6 symmetric reactor core. The [PlaneDeletionGenerator.md] can be employed to trim the 2D full core mesh into the desired symmetrical configuration in this case because the desired cuts lie along existing element boundaries. Alternatively, [HexagonMeshTrimmer.md] could be used to perform this trimming.

### Object

- [PlaneDeletionGenerator.md]

### Geometry Features

- Trimming plane defined by a point and a normal vector
- Elements whose centroids lie "above" (in the direction of the normal vector) the plane will be deleted
- Set new outer boundary ID on sliced lines to be referenced later
- To slice the full core in half

  - +Point+ = $(0, 0, 0)$, +normal+ = $(10, 17.32, 0)$ (60&deg; from +$x$)

- To slice the half core into 1/3

  - +Point+ = $(0, 0, 0)$, +normal+ = $(10, -17.32, 0)$ (-60&deg; from +$x$)

### Notes

- Advanced trimming options are available (see [HexagonMeshTrimmer.md]). Trimming should only be performed along lines of symmetry.

### Example

!media tutorial04_meshing/hpmr_slice.png
       id=tutorial04-hpmr_slice
       caption=1/6 core slice in two steps.
       style=width:50%;display:block;margin-left:auto;margin-right:auto;

!listing reactor_examples/hpmr/hpmr.i
         id=tutorial04-HPMR_9.9_Slice_to_1over6_Core-1
         caption=Slice full core in 1/2 example.
         block=Mesh/coreslice_1

!listing reactor_examples/hpmr/hpmr.i
         id=tutorial04-HPMR_9.9_Slice_to_1over6_Core-2
         caption=Slice 1/2 core in 1/3 example.
         block=Mesh/coreslice_2

## Extrude to 3D

The [AdvancedExtruderGenerator.md] performs 2D to 3D extrusion, and allows users to control elevations through variable extrusion (axial) lengths.  This mesh generator also has features like assigning new subdomain IDs at different axial layers.

### Object

- [AdvancedExtruderGenerator.md]

### Geometry Features

Extrude the 2D $(x,y)$ mesh in +$z$ direction $(0, 0, 1)$

- Split into 3 axial intervals
- Heights for each interval: 20 units, 160 units, 20 units

  - Number of layers in each interval: 1, 8, 1

- Set top/bottom boundary IDs to be referenced later

### Notes

Here we explain the concept of subdomain swaps. By default, when 2D elements are extruded to 3D using [AdvancedExtruderGenerator.md], they retain the same subdomain IDs as their original 2D elements. In the 3D HPMR, distinct subdomain IDs are required for the upper and lower reflector regions, as well as for the core center region. This can be achieved using the subdomain swap function in the [AdvancedExtruderGenerator.md] through the [!param](/Mesh/AdvancedExtruderGenerator/subdomain_swaps) parameter. Each element of the [!param](/Mesh/AdvancedExtruderGenerator/subdomain_swaps) vector contains subdomain remapping information for a specific elevation, with the first element representing the initial extruded elevation. For instance, a 2D moderator mesh with block ID `100` can be extruded to 3D using the following [!param](/Mesh/AdvancedExtruderGenerator/subdomain_swaps): `'100 1000; 100 100; 100 1000'`, where `1000` is the block ID of the reflector. The generated mesh will be defined to be three layers with the upper and bottom meshes of block ID `1000` and the center region maintain the block ID of `100`. The resulting mesh will have three layers, with the top and bottom layers having a block ID of `1000` (reflector), while the central region retains the block ID of `100` (moderator).

### Example

!media tutorial04_meshing/hpmr_extrude.png
       id=tutorial04-hpmr_extrude
       caption=1/6 2D core extruded to 3D.
       style=width:50%;display:block;margin-left:auto;margin-right:auto;

!listing reactor_examples/hpmr/hpmr.i
         id=tutorial04-HPMR_9.10_Extrude_to_3D
         caption=1/6 2D core extruded to 3D.
         block=Mesh/extrude

## Using the Mesh in Downstream Physics Applications (Griffin)

We briefly describe some key steps which are required to use the resulting mesh in Griffin. Namely, block IDs (`subdomain_id`) are referenced in Griffin to assign materials to elements. The external boundary sideset IDs are referenced in Griffin to assign boundary conditions.

### Assignment of Material Properties to Blocks

All blocks in the mesh must be assigned to a material in Griffin. The blocks are depicted in [tutorial04-hpmr_blocks] (left) using different colors for each block. [tutorial04-hpmr_blocks] (right) highlights two specific blocks and their associated material IDs as an example of block IDs that must be referenced in Griffin.

!media tutorial04_meshing/hpmr_blocks.png
       id=tutorial04-hpmr_blocks
       caption=Locations of blocks which are assigned to materials in the Griffin input file.
       style=width:50%;display:block;margin-left:auto;margin-right:auto;

!listing reactor_examples/hpmr/hpmr_griffin_snippet.i
         id=tutorial04-hpmr_griffin_snippet
         caption=Material property assignment example.
         block=Materials

### Assignment of Boundary Conditions to Sidesets

!media tutorial04_meshing/hpmr_sidesets.png
       id=tutorial04-hpmr_sidesets
       caption=The key sidesets which are assigned boundary conditions in the Griffin input file.
       style=width:50%;display:block;margin-left:auto;margin-right:auto;

### Generation of Coarse Mesh in Griffin

We briefly touch on mesh generation for the Coarse Mesh Finite Difference acceleration option in Griffin. There are three options:

- Option 1: Define "coarse" mesh that is identical to fine mesh
- Option 2: Define coarse mesh covering the same geometry as the fine mesh but with a coarser mesh refinement
- Option 3: Define a regular square grid covering the entire mesh domain (and beyond)

Examples of options 2 and 3 are provided in the following images.

!media tutorial04_meshing/hpmr_coarse_mesh_cartesian_stencil_2.png
       id=tutorial04-hpmr_coarse_mesh_cartesian_stencil_2
       caption=Separately generated fine-mesh stencil used for coarse mesh generation of HPMR mesh (left), and HPMR mesh colored by `coarse_element_id`.
       style=width:50%;display:block;margin-left:auto;margin-right:auto;

!media tutorial04_meshing/hpmr_coarse_mesh_cartesian_stencil_1.png
       id=tutorial04-hpmr_coarse_mesh_cartesian_stencil_1
       caption=Cartesian stencil used for coarse mesh generation of HPMR mesh (left), and HPMR mesh colored by `coarse_element_id`.
       style=width:50%;display:block;margin-left:auto;margin-right:auto;

!content pagination previous=tutorial04_meshing/step08_abtr.md
                    next=tutorial04_meshing/step10_rgmb.md
