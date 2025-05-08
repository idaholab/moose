# Example: Homogenous Assembly Fast Reactor Core (ABTR)

!---

## Homogenous Assembly Fast Reactor Core (ABTR)

!row!
!col! width=66%

- We now build a sodium-cooled fast reactor core mesh for the Advanced Burner Test Reactor (ABTR) ([!cite](shemon2015abtr)) using the following key steps:

  - Create homogenized hexagonal assemblies
  - Create dummy assemblies needed for core patterning
  - Combine assemblies into a full core
  - Delete dummy assemblies
  - Extrude 2D mesh to 3D
  - Assign plane-level reporting IDs
  - Rename outer boundary sidesets for later use in Griffin (optional)

!col-end!

!col! width=33%

!media tutorial04_meshing/hom_abtr_stepbystep.png
       style=width:100%;display:block;margin-left:auto;margin-right:auto;

!col-end!

!row-end!

+Hands-on package MOOSE input file+: `tutorials/tutorial04_meshing/doc/listings/reactor_examples/abtr/abtr.i`

!---

## Define Homogeneous Hexagonal Assemblies

!row!
!col! width=66%

- [SimpleHexagonGenerator.md]

- Hexagonal assembly
- Size of 7.3425 (apothem style, center to wall distance, or half-pitch).
- QUAD elements
- Assign unique block ID to each assembly type

- Each assembly type requires its own definition so that different materials can later be assigned to different assemblies (using block id as a differentiating factor)
- Alternatively, using [!param](/Mesh/SimpleHexagonGenerator/element_type) = `TRI` discretizes hexagonal assembly into 6 triangles

!col-end!

!col! width=33%

!media tutorial04_meshing/hom_abtr_assembly_quads.png
       style=width:100%;display:block;margin-left:auto;margin-right:auto;

!col-end!

!row-end!

!listing reactor_examples/abtr/abtr.i
         block=Mesh/control
         link=False

!---

## Define Dummy Assemblies

!row!
!col! width=66%

- [SimpleHexagonGenerator.md]

- Hexagonal assembly
- 7.3425 cm half-pitch
- QUAD elements

- Use a specific ID or name for the dummy in order to delete later in mesh generation process

!col-end!

!col! width=33%

!media tutorial04_meshing/hom_abtr_assembly_quads.png
       style=width:100%;display:block;margin-left:auto;margin-right:auto;

!col-end!

!row-end!

!listing reactor_examples/abtr/abtr.i
         block=Mesh/dummy
         link=False

!---

## Assemble Core Lattice

- [PatternedHexMeshGenerator.md]

- Uses all generated real assemblies and dummy assembly as input

- The parameters [!param](/Mesh/PatternedHexMeshGenerator/id_name), [!param](/Mesh/PatternedHexMeshGenerator/assign_type), and [!param](/Mesh/PatternedHexMeshGenerator/exclude_id) define how the `assembly_id` reporting ID will be generated. We exclude the dummy assemblies from being assigned IDs.

!row!
!col! width=66%

!listing reactor_examples/abtr/abtr.i
         block=Mesh/core
         link=False

!col-end!

!col! width=33%

!media tutorial04_meshing/hom_abtr_2d_core_with_dummies.png
       style=width:100%;display:block;margin-left:auto;margin-right:auto;

!col-end!
!row-end!

!---

## Delete Dummy Assemblies

!row!
!col! width=66%

- [BlockDeletionGenerator.md]

- Remove "dummy" assemblies added for core hex patterning

- Set [!param](/Mesh/BlockDeletionGenerator/new_boundary) to same value as outer boundary in `Mesh/core/external_boundary_name` to update the outer boundary sideset along the location of deleted assemblies

!col-end!

!col! width=33%

!media tutorial04_meshing/hom_abtr_2d_core.png
       style=width:100%;display:block;margin-left:auto;margin-right:auto;

!col-end!

!row-end!

!listing reactor_examples/abtr/abtr.i
         block=Mesh/del_dummy
         link=False

!---

## Extrude 2D core to 3D

- [AdvancedExtruderGenerator.md]

- Extrude 2D mesh to 3D (in +$z$ direction $(0, 0, 1)$)

- Split into multiple intervals, definite heights and number of layers for each
- Set top/bottom boundary IDs to be referenced later

- Assign new block IDs to each axial level using [!param](/Mesh/AdvancedExtruderGenerator/subdomain_swaps)

!row!
!col! width=66%

!listing reactor_examples/abtr/abtr.i
         block=Mesh/extrude
         link=False

!col-end!

!col! width=33%

!media tutorial04_meshing/hom_abtr_3d_core.png
       style=width:100%;display:block;margin-left:auto;margin-right:auto;

!col-end!

!row-end!

!---

## Assign Plane-level Reporting IDs

!row!
!col! width=66%

- [PlaneIDMeshGenerator.md]

- Assign coordinates demarking axial levels in plane_coordinates. These levels should be consistent with how axial levels were defined in [AdvancedExtruderGenerator.md].

!col-end!

!col! width=33%

!media tutorial04_meshing/hom_abtr_3d_core_with_plane_id.png
       style=width:100%;display:block;margin-left:auto;margin-right:auto;

!col-end!

!row-end!

!listing reactor_examples/abtr/abtr.i
         block=Mesh/plane_id
         link=False

!---

## (Optional) Rename Outer Boundary Sidesets

!row!
!col! width=66%

- [RenameBoundaryGenerator.md]

- Griffin requires the outer boundary sidesets to be defined to apply boundary conditions such as vacuum or reflective boundary conditions

!col-end!

!col! width=33%

!media tutorial04_meshing/hom_abtr_3d_sidesets.png
       style=width:100%;display:block;margin-left:auto;margin-right:auto;

!col-end!

!row-end!

!listing reactor_examples/abtr/abtr.i
         block=Mesh/abtr_mesh
         link=False

!---

## Use of ABTR Mesh in Downstream Physics Code (Griffin)

The Reactor Module creates meshes containing blocks of elements (identified by block ID), groups of elements with similar reporting IDs (identified by different reporting IDs such as `pin_id`, `assembly_id`, `depletion_id`), and groups of curves (2D meshes) or faces (3D meshes) called sidesets (identified by sideset ID). In particular, the blocks and sidesets are used in downstream physics codes to assign materials to mesh elements and to assign boundary conditions.

These assignments are discussed here for [Griffin](https://inl.gov/ncrc/code-descriptions/), a MOOSE-based reactor physics code developed under the DOE Nuclear Energy Advanced Modeling and Simulation Program.

!---

### Assignment of Material Properties to Blocks

Griffin's `MixedNeutronicsMaterial` defines the mesh-material mapping explicitly using the subdomain IDs defined on the mesh. Each corresponding material ID defines the cross-section properties for those mesh elements.

The key point is that the block IDs (`subdomain_id`) in the mesh need to be referenced in the Griffin input file in order to map materials to these blocks. A separate `MixedNeutronicsMaterial` should be defined in the Griffin input for each unique material ID pertaining to the input mesh.

!listing reactor_examples/abtr/abtr_griffin_snippet.i
         block=Materials/icore
         link=False

!---

### Assignment of Boundary Conditions to Sidesets

Griffin requires boundary conditions to be applied to all external boundaries of the mesh (generally the top, bottom, and radial periphery for a typical 3D core). Boundary conditions are set in the `TransportSystems` block of Griffin. These outer boundary sidesets must be assigned to the appropriate boundary condition type (e.g.,  `VacuumBoundary`, `ReflectingBoundary`, etc.).

!listing reactor_examples/abtr/abtr_griffin_snippet.i
         block=TransportSystems
         link=False

!---

### Generation of CMFD Mesh in Griffin

We briefly touch on mesh generation for the Coarse Mesh Finite Difference (CMFD) acceleration option in Griffin. There are three options:

- Option 1: Define "coarse" mesh that is identical to fine mesh
- Option 2: Define coarse mesh covering the same geometry as the fine mesh, but with a coarser mesh refinement
- Option 3: Define a regular square grid covering the entire mesh domain (and beyond)

This ABTR example uses option 1, where the CMFD acceleration uses the same mesh as the fine mesh, so no additional mesh generation is performed.

So far, the example shown in this section has relied on explicit region ID mappings for the homogeneous core. Section ["RGMB Example: Heterogeneous to Homogeneous Conversion for Fast Reactor Core"](tutorial04_meshing/presentation/index.md#/11) will explore how the Reactor Geometry Mesh Builder can be used to generate the heterogeneous input mesh and automatically define the homogeneous core mesh from the heterogeneous mesh specifications.

!---

### Output Postprocessing

- [ExtraIDIntegralVectorPostprocessor.md]

- For each [ExtraIDIntegralVectorPostprocessor.md], a separate CSV file is generated to describe the integral variable quantity as a function of each combination of input reporting ID provided. Alternatively, [ExtraIDIntegralReporter.md] can output in JSON file format, which is more suitable for additional data parsing using script languages.

!listing reactor_examples/abtr/abtr_griffin_snippet.i
         block=PowerDensity VectorPostprocessors
         link=False

!---

### Output Postprocessing

- [ExtraIDIntegralVectorPostprocessor.md]

- For each [ExtraIDIntegralVectorPostprocessor.md], a separate CSV file is generated to describe the integral variable quantity as a function of each combination of input reporting ID provided. Alternatively, [ExtraIDIntegralReporter.md] can output in JSON file format, which is more suitable for additional data parsing using script languages.

!row!
!col! width=50%

!media tutorial04_meshing/hom_abtr_vpp_csv.png
       style=width:100%;display:block;margin-left:auto;margin-right:auto;

!col-end!

!col! width=50%

!media tutorial04_meshing/hom_abtr_script_vis.png
       style=width:100%;display:block;margin-left:auto;margin-right:auto;

!col-end!

!row-end!
