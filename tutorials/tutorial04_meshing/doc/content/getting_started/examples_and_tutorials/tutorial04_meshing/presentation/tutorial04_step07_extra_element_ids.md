# Reporting IDs: A Powerful Feature for Assisting with Physics Input and Output Processing

!---

## Why do we need Reporting IDs?

!row!
!col small=12 medium=6 large=8

- In reactor simulations, we want to bookkeep the individual elements belonging to each geometric component

  - Assign material properties to the mesh in different regions
  - Extract integral quantities from the solution in different regions
- Using numerous block IDs just to differentiate regions is not practical or sufficient

  - Using excessive blocks can cause performance degradation in MOOSE
  - Multiple hierarchical levels in geometries (e.g., pin, assembly) cannot be represented with blocks
- Reporting IDs were introduced as a practical solution to this bookkeeping issue

!col small=12 medium=6 large=4

!media tutorial04_meshing/block_vs_reporting_id.png
       style=width:100%;display:block;margin-left:auto;margin-right:auto;

!row-end!

!---

## What are Reporting IDs?

- Reporting IDs are extra integer ID tags assigned on each element of the mesh

  - A reporting ID consists of a name (e.g. pin_id, assembly_id) and an assigned value (e.g. 1, 2, 3...)
  - A reporting ID designates association with a specific reactor component or zone, such as pin ID or assembly ID
  - An element may have multiple reporting IDs to track different information (e.g., pin number, assembly number, plane number).
- How do we get reporting IDs on mesh elements?

  - The automatic assignment of reporting IDs to elements in a mesh is provided through several mesh generators that "understand" the concept of pins, assemblies, planes, etc.
  - There is no need to provide physical locations or coordinates of elements in order to assign IDs

!---

## How can we use reporting IDs?

- Reporting IDs can be used to assign material properties
- Reporting IDs can be used to create additional unique zones (e.g. depletion zones)
- Reporting IDs can be leveraged to post-process solution data into tables by using the [ExtraIDIntegralVectorPostprocessor.md]. This postprocessor integrates the solution based on reporting IDs. Component-wise values such as pin-by-pin power distribution can be easily yielded by specifying integration over pin and assembly reporting IDs to this postprocessor.

!---

## Applying Reporting IDs for Cartesian and Hexagonal Lattices

- [PatternedCartesianMeshGenerator.md] (Cartesian)
- [PatternedHexMeshGenerator.md] (Hexagonal)

- Assign reporting IDs for input geometric components (pins or assemblies) during lattice mesh generations.
- Supports the following numbering schemes (set with [!param](/Mesh/PatternedHexMeshGenerator/assign_type)):

  - `cell` (default): Assign unique IDs for each component in the lattice in sequential order (begins at 0 in the top left corner of the pattern).
  - `pattern`: assign a different ID for each unique input component
  - `manual`: use a manual numbering scheme provided by user
- When assembly duct regions are present, these regions are numbered sequentially starting from the inner-most region to the outer-most region.

- Pin and Assembly IDs are applied during creations of assemblies and core, respectively.

!---

### Cell Pattern Example

!row!
!col small=12 medium=6 large=8

!media tutorial04_meshing/eeid_cart_hex_examples.png
       style=width:100%;display:block;margin-left:auto;margin-right:auto;

!col small=12 medium=6 large=4

!listing modules/reactor/test/tests/meshgenerators/reporting_id/cartesian_id/patterned_cartesian_core_reporting_id.i
         block=Mesh
         link=False

!row-end!

!---

### Alternative Pattern Example

- Supports other numbering schemes:

  - [!param](/Mesh/PatternedHexMeshGenerator/assign_type) = `pattern`: Assign IDs identical to the already-provided [!param](/Mesh/PatternedHexMeshGenerator/pattern) array
  - [!param](/Mesh/PatternedHexMeshGenerator/assign_type) = `manual`: Assign IDs based on a user-defined mapping in the optional [!param](/Mesh/PatternedHexMeshGenerator/id_pattern) array, which may differ from the required [!param](/Mesh/PatternedHexMeshGenerator/pattern) array

!row!
!col small=12 medium=6 large=8

!listing base_mesh_generators/alternative_pattern_reporting_id.i
         block=Mesh
         link=False

!col small=12 medium=6 large=4

!media tutorial04_meshing/eeid_assign_type_example.png
       style=width:100%;display:block;margin-left:auto;margin-right:auto;

!row-end!

!---

## Applying Reporting IDs for Axial Plane

!row!
!col small=12 medium=6 large=8

- [PlaneIDMeshGenerator.md]

- Apply reporting IDs between axial planes in an already extruded mesh

- Only applicable for extruded geometries where the concept of axial layers (in $x$, $y$, or $z$ directions) is valid
- The input mesh to this mesh generator should be 3D (this mesh generator does not perform the extrusion itself)
- Unique IDs can be assigned between axial planes (coarse approach) or also to each unique sublayer defined by axial subintervals between the planes (fine approach)

!col small=12 medium=6 large=4

!media tutorial04_meshing/eeid_plane_id_examples.png
       style=width:100%;display:block;margin-left:auto;margin-right:auto;

!row-end!

!listing base_mesh_generators/plane_id.i
         block=Mesh/CORE_3D
         link=False

!---

## Applying Depletion IDs

!row!
!col small=12 medium=6 large=8

- [DepletionIDGenerator.md]

- Automatically assign depletion zones based on existing unique combination of reporting IDs and material ID
- Easily control the fidelity of depletion zones based on the other reporting IDs already in the mesh (including block and material IDs)

- For a pin-level depletion case, the depletion IDs for the entire domain can be specified by finding unique combinations of assembly, pin, and material IDs
- By additionally including ring and sector IDs accessible through [PolygonConcentricCircleMeshGenerator.md], depletion zones can be defined within the pin itself

!col small=12 medium=6 large=4

!media tutorial04_meshing/eeid_depletion_id_example.png
       style=width:100%;display:block;margin-left:auto;margin-right:auto;

!row-end!

!listing base_mesh_generators/depletion_id.i
         block=Mesh/depletion_id
         link=False

!---

## Querying Output Data using Reporting IDs

!row!
!col small=12 medium=6 large=8

- [ExtraIDIntegralVectorPostprocessor.md]
- [ExtraIDIntegralReporter.md]

- Integrates solution variables over zones identified by combinations of reporting IDs

- [ExtraIDIntegralVectorPostprocessor.md] exports the post-processed results in CSV file format
- [ExtraIDIntegralReporter.md], based on the MOOSE reporting system, can output in JSON file format

!col small=12 medium=6 large=4

!media tutorial04_meshing/eeid_reporting_id_vpp_example.png
       style=width:100%;display:block;margin-left:auto;margin-right:auto;

!row-end!

!listing base_mesh_generators/reporting_id_vpp.i
         block=VectorPostprocessors
         link=False
