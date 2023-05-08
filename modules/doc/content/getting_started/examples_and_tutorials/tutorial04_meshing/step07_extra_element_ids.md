# Reporting IDs: A Powerful Feature for Assisting with Physics Input and Output Processing

When using an unstructured finite element mesh format for reactor analysis, it can be challenging to identify which individual elements belong to each geometric component, which is necessary for assigning properties and for extracting integral quantities from the output solution. The block ID (also called subdomain ID) has been used in the past for bookkeeping of elements, which requires each bookkeeping region to have a different block ID. If there are multiple hierarchical levels in geometries (e.g., pin, assembly), block ID is not sufficient to track all the associations of a given element association. Additionally, using an excessive number of blocks in a mesh can cause performance degradation in MOOSE compared to using only a few blocks. Reporting IDs were introduced as a practical solution to this bookkeeping issue.  Reporting IDs are extra integer ID tags assigned on each element which designate association with a specific reactor component or zone, such as pin ID or assembly ID. Multiple reporting IDs may be assigned on each element to track different information (e.g., pin number, assembly number, plane number).

The automatic assignment of reporting IDs to elements in a mesh is provided through several mesh generators, as described in the following sections. Collections of elements forming pins and assemblies are identifiable without providing information about their physical location since the mesh generators themselves understand the concept of pins and assemblies.

Reporting IDs can be leveraged to post-process solution data into tables by using the [ExtraIDIntegralVectorPostprocessor.md]. This postprocessor integrates the solution based on reporting IDs. Component-wise values such as pin-by-pin power distribution can be easily yielded by specifying integration over pin and assembly reporting IDs to this postprocessor.

## Applying Reporting IDs for Cartesian and Hexagonal Lattices

In addition to being used to stitch together Cartesian and hexagonal grids from smaller unit meshes, [PatternedCartesianMeshGenerator.md] and [PatternedHexMeshGenerator.md] can also assign reporting IDs to the mesh during the stitching process. When using these mesh generators to stitch pins into an assembly, a pin reporting ID (e.g., `pin_id`) is relevant.  When stitching assemblies into a core, an assembly reporting ID (e.g., `assembly_id`) is relevant.

Since the mesh generators understand the pin or assembly pattern, IDs can be applied in one of several ways: `cell` (default), `pattern`, or `manual`. In the `cell` numbering pattern, each cell is given a unique ID starting with 0 in the upper left-hand corner of the hexagon grid, increasing monotonically from left to right, top to bottom up until N-1 for the final pin location. Duct regions are assigned reporting IDs starting with the next available integer following pin region assignment. Certain regions can be excluded from being labeled with an ID, for example dummy regions that will later be deleted. This can be accommodated by listing mesh objects in the [!param](/Mesh/PatternedHexMeshGenerator/exclude_id) parameter. Usage of this parameter is helpful to retain sequential numbering when dummy regions are later deleted.

### Object

- [PatternedCartesianMeshGenerator.md] (Cartesian)
- [PatternedHexMeshGenerator.md] (Hexagonal)

### Features

- Assign reporting IDs for input geometric components (pins or assemblies) during lattice mesh generations.
- Supports the following numbering schemes (set with [!param](/Mesh/PatternedHexMeshGenerator/assign_type)):

  - `cell` (default): Assign unique IDs for each component in the lattice in sequential order (begins at 0 in the top left corner of the pattern).
  - `pattern`: assign a different ID for each unique input component
  - `manual`: use a manual numbering scheme provided by user
- When assembly duct regions are present, these regions are numbered sequentially starting from the inner-most region to the outer-most region.

### Notes

- Pin and Assembly IDs are applied during creations of assemblies and core, respectively.

### Cell Pattern Example

!media tutorial04_meshing/eeid_cart_hex_examples.png
       id=tutorial04-eeid_cart_hex_examples
       caption=Typical Cartesian (left) and hexagonal (right) cores, colored by Pin ID (top) and Assembly ID (bottom).
       style=width:50%;display:block;margin-left:auto;margin-right:auto;

!listing modules/reactor/test/tests/meshgenerators/reporting_id/cartesian_id/patterned_cartesian_core_reporting_id.i
         id=tutorial04-core_reporting_id_cart.i
         caption=Cell pattern reporting ID example for Cartesian lattices.
         block=Mesh

!listing modules/reactor/test/tests/meshgenerators/reporting_id/hexagonal_id/core_reporting_id.i
         id=tutorial04-core_reporting_id_hex.i
         caption=Cell pattern reporting ID example for hexagonal lattices.
         block=Mesh

### Alternative Pattern Example

Supports other numbering schemes:

- [!param](/Mesh/PatternedHexMeshGenerator/assign_type) = `pattern`: Assign IDs identical to the already-provided [!param](/Mesh/PatternedHexMeshGenerator/pattern) array
- [!param](/Mesh/PatternedHexMeshGenerator/assign_type) = `manual`: Assign IDs based on a user-defined mapping in the optional [!param](/Mesh/PatternedHexMeshGenerator/id_pattern) array, which may differ from the required [!param](/Mesh/PatternedHexMeshGenerator/pattern) array

!media tutorial04_meshing/eeid_assign_type_example.png
       id=tutorial04-eeid_assign_type_example
       caption=Pattern (left) and Manual (right) Numbering scheme, colored by Pin ID.
       style=width:50%;display:block;margin-left:auto;margin-right:auto;

!listing base_mesh_generators/alternative_pattern_reporting_id.i
         id=tutorial04-alternative_pattern_reporting_id.i
         caption=Alternative pattern reporting ID example.
         block=Mesh

## Applying Reporting IDs for Axial Plane

Users often wish to postprocess solutions along the axial dimension. The [PlaneIDMeshGenerator.md] applies reporting IDs to axial planes in an already extruded mesh. To be clear, this mesh generator does +NOT+ extrude the mesh: an extruded mesh must be provided as input.

### Object

- [PlaneIDMeshGenerator.md]

### Features

- Apply reporting IDs between axial planes in an already extruded mesh

### Notes

- Only applicable for extruded geometries where the concept of axial layers (in $x$, $y$, or $z$ directions) is valid
- The input mesh to this mesh generator should be 3D (this mesh generator does not perform the extrusion itself)
- Unique IDs can be assigned between axial planes (coarse approach) or also to each unique sublayer defined by axial subintervals between the planes (fine approach)

### Example

!media tutorial04_meshing/eeid_plane_id_examples.png
       id=tutorial04-eeid_plane_id_examples
       caption=Simplified extruded cores with (left) and without (right) subinterval numberings on center region, colored by plane IDs.
       style=width:50%;display:block;margin-left:auto;margin-right:auto;

!listing base_mesh_generators/plane_id.i
         id=tutorial04-plane_id.i
         caption=Plane ID example with subinterval numbering. Removing the [!param](/Mesh/PlaneIDMeshGenerator/num_ids_per_plane) parameter will produce the same core without subinterval numbering.
         block=Mesh/CORE_3D

## Applying Depletion IDs

The [DepletionIDGenerator.md] is useful for defining neutronics depletion zones. The fidelity of the depletion zones (pin-level, assembly-level, or even intra-pin zones) can be handled.

### Object

- [DepletionIDGenerator.md]

### Features

- Automatically assign depletion zones based on existing unique combination of reporting IDs and material ID
- Easily control the fidelity of depletion zones based on the other reporting IDs already in the mesh (including block and material IDs)

### Notes

- For a pin-level depletion case, the depletion IDs for the entire domain can be specified by finding unique combinations of assembly, pin, and material IDs
- By additionally including ring and sector IDs accessible through [PolygonConcentricCircleMeshGenerator.md], depletion zones can be defined within the pin itself

### Example

!media tutorial04_meshing/eeid_depletion_id_example.png
       id=tutorial04-eeid_depletion_id_example
       caption=Generating depletion ID using pin and assembly reporting IDs.
       style=width:50%;display:block;margin-left:auto;margin-right:auto;

!listing base_mesh_generators/depletion_id.i
         id=tutorial04-depletion_id.i
         caption=Depletion ID example.
         block=Mesh/depletion_id

## Querying Output Data using Reporting IDs

After performing a physics simulation using a mesh with reporting IDs, the solution variables may be post-processed into useful integrals using the reporting IDs. [ExtraIDIntegralVectorPostprocessor.md] and [ExtraIDIntegralReporter.md] both integrate solution variables over zones identified by unique combinations of reporting IDs (controlled by the user). The user can easily retrieve quantities from an unstructured mesh such as "pin power of pin 6 in assembly 2 on axial layer 3" by using the `pin_id`, `assembly_id`, and `plane_id` as the integration criteria.

### Objects

- [ExtraIDIntegralVectorPostprocessor.md]
- [ExtraIDIntegralReporter.md]

### Features

- Integrates solution variables over zones identified by combinations of reporting IDs

### Notes

- [ExtraIDIntegralVectorPostprocessor.md] exports the post-processed results in CSV file format
- [ExtraIDIntegralReporter.md], based on the MOOSE reporting system, can output in JSON file format

### Example

!media tutorial04_meshing/eeid_reporting_id_vpp_example.png
       id=tutorial04-eeid_reporting_id_vpp_example
       caption=Example of vector post-processing utilizing reporting IDs.
       style=width:75%;display:block;margin-left:auto;margin-right:auto;

!listing base_mesh_generators/reporting_id_vpp.i
         id=tutorial04-reporting_id_vpp.i
         caption=Vector post-processing example.
         block=VectorPostprocessors

!content pagination previous=tutorial04_meshing/step06_common_ops.md
                    next=tutorial04_meshing/step08_abtr.md
