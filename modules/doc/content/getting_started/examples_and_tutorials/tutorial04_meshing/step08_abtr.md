# Example: Homogenous Assembly Fast Reactor Core (ABTR)

We now build a sodium-cooled fast reactor core mesh for the Advanced Burner Test Reactor (ABTR) ([!cite](shemon2015abtr)) using the following key steps.

1. Create homogenized hexagonal assemblies
2. Create dummy assemblies needed for core patterning
3. Combine assemblies into a full core
4. Delete dummy assemblies
5. Extrude 2D mesh to 3D
6. Assign plane-level reporting IDs
7. Rename outer boundary sidesets for later use in Griffin (optional)

The step-by-step instructions to build the mesh are followed by notes on how to use the mesh in Griffin.

!media tutorial04_meshing/hom_abtr_stepbystep.png
       id=tutorial04-hom_abtr_stepbystep
       caption=Visualization of meshing steps to build the 3D ABTR core with homogenized assemblies.
       style=width:75%;display:block;margin-left:auto;margin-right:auto;

## Define Homogeneous Hexagonal Assemblies

The first step is to define unique mesh objects for each different assembly type (fuel, control, shield, reflector, dummy). Since this is a homogenized model, we use [SimpleHexagonGenerator.md] to create each assembly mesh.

### Object

- [SimpleHexagonGenerator.md]

### Geometry Features

- Hexagonal assembly
- Size of 7.3425 (apothem style, meaning the center to wall distance, or half-pitch). No units are implied in the mesh, units will be interpreted by the physics code (generally cm for Griffin)
- QUAD elements
- Assign unique block ID to each assembly type

### Notes

- Each assembly type requires its own definition so that different materials can later be assigned to different assemblies (using block id as a differentiating factor)
- Alternatively, using [!param](/Mesh/SimpleHexagonGenerator/element_type) = `TRI` discretizes hexagonal assembly into 6 triangles

### Example

!media tutorial04_meshing/hom_abtr_assembly_quads.png
       id=tutorial04-hom_abtr_assembly_quads
       caption=Homogeneous assembly defined with 2 quadrilateral elements.
       style=width:50%;display:block;margin-left:auto;margin-right:auto;

!listing reactor_examples/abtr/abtr.i
         id=tutorial04-abtr_mesh_quads
         caption=Input to create a single homogenized assembly.
         block=Mesh/control

## Define Dummy Assemblies

When we later stitch assemblies into a hexagonal core, the stitcher requires a "perfect" hexagonal pattern. We need to define dummy assemblies to fill the "empty" spots in the perimeter of the core.

### Object

- [SimpleHexagonGenerator.md]

### Geometry Features

- Hexagonal assembly
- 7.3425 cm half-pitch should be input as 7.3425 units "apothem style"
- QUAD elements

### Notes

- Use a specific ID or name for the dummy in order to delete later in mesh generation process

### Example Input

!listing reactor_examples/abtr/abtr.i
         id=tutorial04-abtr_mesh_dummy
         caption=Homogeneous dummy example.
         block=Mesh/dummy

## Assemble Core Lattice

Now that assemblies have been defined, we stitch the assemblies into a perfect hexagonal pattern to form the initial core. Dummy assemblies are placed in the empty slots and will be deleted in the next step. During this step, reporting IDs called `assembly_id` are also assigned to each assembly.

### Object

- [PatternedHexMeshGenerator.md]

### Geometry Features

- Uses all generated real assemblies and dummy assembly as input

### Notes

- The parameters [!param](/Mesh/PatternedHexMeshGenerator/id_name), [!param](/Mesh/PatternedHexMeshGenerator/assign_type), and [!param](/Mesh/PatternedHexMeshGenerator/exclude_id) define how the `assembly_id` reporting ID will be generated. We exclude the dummy assemblies from being assigned IDs.

### Example

!media tutorial04_meshing/hom_abtr_2d_core_with_dummies.png
       id=tutorial04-hom_abtr_2d_core_with_dummies
       caption=2D core including dummy assemblies in the "empty" locations.
       style=width:50%;display:block;margin-left:auto;margin-right:auto;

!listing reactor_examples/abtr/abtr.i
         id=tutorial04-abtr_mesh_core
         caption=Homogeneous core example.
         block=Mesh/core

## Delete Dummy Assemblies

Dummy assemblies were only included to facilitate the core pattern generation and are not part of the final core. They now need to be deleted.

### Object

- [BlockDeletionGenerator.md]

### Geometry Features

- Remove "dummy" assemblies added for core hex patterning

### Notes

- Set [!param](/Mesh/BlockDeletionGenerator/new_boundary) to same value as outer boundary in `Mesh/core/external_boundary_name` to update the outer boundary sideset along the location of deleted assemblies

### Example

!media tutorial04_meshing/hom_abtr_2d_core.png
       id=tutorial04-hom_abtr_2d_core
       caption=2D core after deletion of dummy assemblies colored by "subdomain_id"(left) and "assembly_id" (right).
       style=width:50%;display:block;margin-left:auto;margin-right:auto;

!listing reactor_examples/abtr/abtr.i
         id=tutorial04-abtr_mesh_dummy_del
         caption=Homogeneous dummy deletion example.
         block=Mesh/del_dummy

## Extrude 2D core to 3D

The [AdvancedExtruderGenerator.md] can be used to perform 2D-to-3D extrusion, allowing users to control elevations through variable extrusion (axial) lengths, axial elements, separate subdomains, additional element integers, and boundaries defined at various elevations.

### Object

- [AdvancedExtruderGenerator.md]

### Geometry Features

- Extrude 2D mesh to 3D

  - Extrude in +$z$ direction $(0, 0, 1)$

- Split into multiple intervals, definite heights and number of layers for each
- Set top/bottom boundary IDs to be referenced later

### Notes

- Assign new block IDs to each axial level using [!param](/Mesh/AdvancedExtruderGenerator/subdomain_swaps). This will allow you to later assign different materials on each axial level
- [!param](/Mesh/AdvancedExtruderGenerator/bottom_boundary) and [!param](/Mesh/AdvancedExtruderGenerator/top_boundary) parameters are used to assign the boundary IDs of the bottom and top boundary sidesets

### Example

!media tutorial04_meshing/hom_abtr_3d_core.png
       id=tutorial04-hom_abtr_3d_core
       caption=3D core after extrusion, colored by subdomain IDs.
       style=width:50%;display:block;margin-left:auto;margin-right:auto;

!listing reactor_examples/abtr/abtr.i
         id=tutorial04-abtr_mesh_extrude
         caption=Homogeneous extrude example.
         block=Mesh/extrude

## Assign Plane-level Reporting IDs

In order to facilitate output processing, we assign `plane_id` reporting IDs to indicate which elements on the 3D mesh belong to the same axial plane.

### Object

- [PlaneIDMeshGenerator.md]

### Geometry Features

- Assign coordinates demarking axial levels in plane_coordinates. These levels should be consistent with how axial levels were defined in [AdvancedExtruderGenerator.md].

### Example

!media tutorial04_meshing/hom_abtr_3d_core_with_plane_id.png
       id=tutorial04-hom_abtr_3d_core_with_plane_id
       caption=3D ABTR colored by "plane_id" reporting ID.
       style=width:50%;display:block;margin-left:auto;margin-right:auto;

!listing reactor_examples/abtr/abtr.i
         id=tutorial04-abtr_mesh_plane_id
         caption=Homogeneous plane ID example.
         block=Mesh/plane_id

## (Optional) Rename Outer Boundary Sidesets

Since [AdvancedExtruderGenerator.md] requires top and bottom boundary sidesets be defined using numeric IDs, we can assign a name (string) such as `'top'` and `'bottom'` to these sidesets for easier reference in physics applications.

### Object

- [RenameBoundaryGenerator.md]

### Notes

- Griffin requires the outer boundary sidesets to be defined to apply boundary conditions such as vacuum or reflective boundary conditions

### Example

!media tutorial04_meshing/hom_abtr_3d_sidesets.png
       id=tutorial04-hom_abtr_3d_sidesets
       caption=Sideset names and locations of the ABTR core mesh.
       style=width:50%;display:block;margin-left:auto;margin-right:auto;

!listing reactor_examples/abtr/abtr.i
         id=tutorial04-abtr_mesh_plane_rename
         caption=Homogeneous boundary rename example.
         block=Mesh/abtr_mesh

## Use of ABTR Mesh in Downstream Physics Code (Griffin)

The Reactor Module creates meshes containing blocks of elements (identified by block ID), groups of elements with similar reporting IDs (identified by different reporting IDs such as `pin_id`, `assembly_id`, `depletion_id`), and groups of curves (2D meshes) or faces (3D meshes) called sidesets (identified by sideset ID). In particular, the blocks and sidesets are used in downstream physics codes to assign materials to mesh elements and to assign boundary conditions.

These assignments are discussed here for [Griffin](https://inl.gov/ncrc/code-descriptions/), a MOOSE-based reactor physics code developed under the DOE Nuclear Energy Advanced Modeling and Simulation Program.

### Assignment of Material Properties to Blocks

Griffin's `MixedNeutronicsMaterial` defines the mesh-material mapping explicitly using the subdomain IDs defined on the mesh. Each corresponding material ID defines the cross-section properties for those mesh elements.

The key point is that the block IDs (`subdomain_id`) in the mesh need to be referenced in the Griffin input file in order to map materials to these blocks. A separate `MixedNeutronicsMaterial` should be defined in the Griffin input for each unique material ID pertaining to the input mesh.

!listing reactor_examples/abtr/abtr_griffin_snippet.i
         id=tutorial04-abtr_griffin_snippet
         caption=Homogeneous Griffin block assignment example.
         block=Materials/icore

### Assignment of Boundary Conditions to Sidesets

Griffin requires boundary conditions to be applied to all external boundaries of the mesh (generally the top, bottom, and radial periphery for a typical 3D core). Boundary conditions are set in the `TransportSystems` block of Griffin. These outer boundary sidesets must be assigned to the appropriate boundary condition type (e.g.,  `VacuumBoundary`, `ReflectingBoundary`, etc.).

!listing reactor_examples/abtr/abtr_griffin_snippet.i
         id=tutorial04-abtr_griffin_snippet-transport
         caption=Homogeneous Griffin Transport Systems example.
         block=TransportSystems

### Generation of CMFD Mesh in Griffin

We briefly touch on mesh generation for the Coarse Mesh Finite Difference (CMFD) acceleration option in Griffin. There are three options:

- Option 1: Define "coarse" mesh that is identical to fine mesh
- Option 2: Define coarse mesh covering the same geometry as the fine mesh, but with a coarser mesh refinement
- Option 3: Define a regular square grid covering the entire mesh domain (and beyond)

This ABTR example uses option 1, where the CMFD acceleration uses the same mesh as the fine mesh, so no additional mesh generation is performed.

### Output Postprocessing

MOOSE provides various ways to post-process mesh-based data. The reporting IDs applied to the mesh to designate pin, assembly, and axial zones can be leveraged to post-process reactor core data into tables printing axial pin power distributions by assembly, for example. In this example, integral powers defined as a function of `assembly_id` and `plane_id` allow for easy postprocessing of radial and axial powers.

### Object

- [ExtraIDIntegralVectorPostprocessor.md]

### Notes

- For each [ExtraIDIntegralVectorPostprocessor.md], a separate CSV file is generated to describe the integral variable quantity as a function of each combination of input reporting ID provided. Alternatively, [ExtraIDIntegralReporter.md] can output in JSON file format, which is more suitable for additional data parsing using script languages.

### Example

!media tutorial04_meshing/hom_abtr_vpp_csv.png
       id=tutorial04-hom_abtr_vpp_csv
       caption=CSV formatted output data from assembly_power_2d (left), axial_power (middle), and assembly_power_3d (right) VectorPostprocessors. Non-zero rows have been filtered out.
       style=width:50%;display:block;margin-left:auto;margin-right:auto;

!media tutorial04_meshing/hom_abtr_script_vis.png
       id=tutorial04-hom_abtr_script_vis
       caption=Visualization of normalized axially integrated assembly power density for homogeneous ABTR core, processed from output CSV VectorPostprocessor data. This figure was generated with a custom script taking CSV-formatted power data as input.
       style=width:50%;display:block;margin-left:auto;margin-right:auto;

!listing reactor_examples/abtr/abtr_griffin_snippet.i
         id=tutorial04-abtr_griffin_snippet-postprocessors
         caption=Homogeneous Griffin postprocessor example.
         block=PowerDensity VectorPostprocessors

!bibtex bibliography !!include to make sure next/previous are last on page

!content pagination previous=tutorial04_meshing/step07_extra_element_ids.md
                    next=tutorial04_meshing/step09_hpmr.md
