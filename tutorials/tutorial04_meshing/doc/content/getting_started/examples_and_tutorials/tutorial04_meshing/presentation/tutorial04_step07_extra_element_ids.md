# Reporting IDs: A Powerful Feature for Assisting with Physics Input and Output Processing

!---

When using an unstructured finite element mesh format for reactor analysis, it can be challenging to identify which individual elements belong to each geometric component, which is necessary for assigning properties and for extracting integral quantities from the output solution. The block ID (also called subdomain ID) has been used in the past for bookkeeping of elements, which requires each bookkeeping region to have a different block ID. If there are multiple hierarchical levels in geometries (e.g., pin, assembly), block ID is not sufficient to track all the associations of a given element association. Additionally, using an excessive number of blocks in a mesh can cause performance degradation in MOOSE compared to using only a few blocks. Reporting IDs were introduced as a practical solution to this bookkeeping issue.  Reporting IDs are extra integer ID tags assigned on each element which designate association with a specific reactor component or zone, such as pin ID or assembly ID. Multiple reporting IDs may be assigned on each element to track different information (e.g., pin number, assembly number, plane number).

The automatic assignment of reporting IDs to elements in a mesh is provided through several mesh generators, as described in the following sections. Collections of elements forming pins and assemblies are identifiable without providing information about their physical location since the mesh generators themselves understand the concept of pins and assemblies.

Reporting IDs can be leveraged to post-process solution data into tables by using the [ExtraIDIntegralVectorPostprocessor.md]. This postprocessor integrates the solution based on reporting IDs. Component-wise values such as pin-by-pin power distribution can be easily yielded by specifying integration over pin and assembly reporting IDs to this postprocessor.

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

  - [!param](/Mesh/PatternedHexMeshGenerator/assign_type) = `pattern`: Assign IDs based on the ID of the input file component as listed in the pattern
  - [!param](/Mesh/PatternedHexMeshGenerator/assign_type) = `manual`: Assign IDs based on a user-defined mapping defined in an array [!param](/Mesh/PatternedHexMeshGenerator/id_pattern)

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
