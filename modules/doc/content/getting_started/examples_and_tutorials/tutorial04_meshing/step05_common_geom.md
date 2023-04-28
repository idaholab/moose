# Frequently Used Reactor Geometries and Corresponding Mesh Generators

This section lists frequently used hexagonal-based geometries for reactor cores and their associated mesh generator objects. All the following are considered "base" mesh generators which expose all input options including control of block (subdomain) numbering. While sidesets can also be numbered, the outer boundary sideset is automatically assigned to sideset 10000.

A second set of mesh generators (Reactor Geometry Mesh Builder mesh generators) is available for regular hexagonal and Cartesian geometry and provides a reduced set of input options, removes any mention of block (subdomain) numbering, and instead allows the user to specify materials directly on the mesh. These are covered in another [Chapter](step10_rgmb.md).

## Pin Cell

Pin cells form the basis of most reactor geometries and consist of concentric circular regions (fuel, clad, etc.) surrounded by a coolant zone. Sometimes, a ducted structural region surrounds the coolant zone. Due to the prevalence of pin cells in both hexagonal and Cartesian geometry, a dedicated mesh generator, [PolygonConcentricCircleMeshGenerator.md], was created for pin cell generation of any regular polygon (three sides or more). It is important to note that the meshed circular areas can be forced to preserve area regardless of the discretization fidelity. This is very useful for mesh convergence studies in which changing the discretization could add or remove fuel from the problem. Preserving the fuel area or volume using the [!param](/Mesh/PolygonConcentricCircleMeshGenerator/preserve_volumes) parameter is highly recommended. This mesh generator can also be used when there is no fuel pin (e.g., an empty duct).

### Object

- [PolygonConcentricCircleMeshGenerator.md]

### Geometry Features

- 2D Cartesian or hexagonal unit pin cell, including fuel, clad, coolant and (optional) ducted regions on periphery

### Notes

- The pin cell size may be provided as the apothem (center-to-flat distance which is the pin "half-pitch") or radius (center-to-vertex distance) depending on the setting in [!param](/Mesh/PolygonConcentricCircleMeshGenerator/polygon_size_style). Apothem (half-pitch) is the default style.
- Permits either quadrilateral or triangular elements in the pin center region (quad fuel and tri fuel regions must have different block IDs)
- Fuel area preservation using the [!param](/Mesh/PolygonConcentricCircleMeshGenerator/preserve_volumes) parameter
- Boundary layer meshing; radial mesh biasing
- Different azimuthal discretization possible per pin cell face

### Example

!media tutorial04_meshing/base_ex_pccmg.png
       id=tutorial04-base_ex_pccmg
       caption=[PolygonConcentricCircleMeshGenerator.md] is capable of generating general polygon meshes as well as Cartesian and hexagonal pins.
       style=width:50%;display:block;margin-left:auto;margin-right:auto;

!listing base_mesh_generators/common_geo.i
         id=tutorial04-common_geo_hex_1
         caption=Pin cell example input.
         block=Mesh/hex_1

## Assembly (Homogenized)

Assemblies are one level higher than pins in the hierarchical lattice geometry structure. They consist of multiple pins laid out in a lattice, but sometimes, these pins are not explicitly represented. The [SimpleHexagonGenerator.md] mesh generator is appropriate for coarse, homogenized assembly approaches. Three modes are available for discretization: +TRI+ (6 triangles), +QUAD+ (2 quadrilaterals) and +HYBRID+ (6 triangles surrounded by layers of quadrilaterals). The +HYBRID+ method can be used when additional radial discretization is needed compared to the other two options. However, there is no additional azimuthal discretization available. [PolygonConcentricCircleMeshGenerator.md] should be used in that case. The hexagon will be discretized using the +TRI+ mode by default if no discretization scheme is specified.

### Object

- [SimpleHexagonGenerator.md]

### Geometry Features

- 2D Hexagonal unit pin cell with coarse mesh discretization

### Notes

- Three modes are available to discretize the hexagon:

  - +TRI+: 6 triangles (default)
  - +QUAD+: 2 quadrilateral elements
  - +HYBRID+: 6 triangles + 6 \* number of layers of quadrilateral elements

### Example

!media tutorial04_meshing/base_ex_shg.png
       id=tutorial04-base_ex_shg
       caption=Example output of the TRI, QUAD, and HYBRID (one quadrilateral layer) mode, respectively, of [SimpleHexagonGenerator.md].
       style=width:50%;display:block;margin-left:auto;margin-right:auto;

!listing base_mesh_generators/common_geo.i
         id=tutorial04-sim_hex.i
         caption=Homogenized assembly example input.
         block=Mesh/hex_simple

## Assembly (with multiple heterogeneous pins)

Assemblies with multiple heterogeneous pins are a very common reactor geometry component. The [PatternedHexMeshGenerator.md] creates an assembly by placing pre-defined pins into a user-defined lattice, and then applying optional background coolant and duct regions. Here we describe only the hexagonal assembly mesh generator. A Cartesian assembly mesh generator, [PatternedCartesianMeshGenerator.md], is also available.

### Object

- [PatternedHexMeshGenerator.md]
- (Cartesian sibling -- [PatternedCartesianMeshGenerator.md])

### Geometry Features

- 2D Regular assembly with uniform pin pitch
- Optional background and duct regions.

### Notes

- When generating an assembly mesh using [PatternedHexMeshGenerator.md], be sure to set [!param](/Mesh/PatternedHexMeshGenerator/generate_core_metadata) as `false`. This tells MOOSE that the constructed object is an assembly rather than a core (and consequently permits some operations downstream differently than if it were a core).
- When generating an assembly mesh, [PolygonConcentricCircleMeshGenerator.md] objects which define the hexagonal unit pin cells are generally used as inputs.
- The input pattern will be automatically rotated 90 degrees counterclockwise into a "vertex up" position unless otherwise specified with [!param](/Mesh/PatternedHexMeshGenerator/rotate_angle). This enables an assembly mesh to be patterned into a core directly (rows of assemblies should be oriented vertex up).

### Example

!media tutorial04_meshing/base_ex_phmg_assm.png
       id=tutorial04-base_ex_phmg_assm
       caption=A schematic showing an assembly mesh generated by [PatternedHexMeshGenerator.md].
       style=width:50%;display:block;margin-left:auto;margin-right:auto;

!listing base_mesh_generators/common_geo.i
         id=tutorial04-common_geo_pattern_assm
         caption=Heterogeneous assembly example input.
         block=Mesh/pattern_assm

## Assembly (control drum, duct-heterogeneous, or single pin)

Another type of assembly in a reactor core is an assembly with a single pin (such as a rotating control drum), or an assembly with no pins explicitly represented (such as a duct-heterogeneous assembly representation). The [HexagonConcentricCircleAdaptiveBoundaryMeshGenerator.md] is typically used to create these types of assembly meshes. It creates an object that looks just like an output of [PolygonConcentricCircleMeshGenerator.md], but has "assembly" metadata and additionally can have different discretizations on each side which are adaptively meshed to match other neighboring assembly meshes.

### Object

- [HexagonConcentricCircleAdaptiveBoundaryMeshGenerator.md]
- (Cartesian sibling -- [CartesianConcentricCircleAdaptiveBoundaryMeshGenerator.md])

### Geometry Features

- Hexagonal mesh with assembly metadata
- Can optionally match other assembly meshes' external boundary to enable stitching to generate core meshes.

### Notes

- It can be used without specifying [!param](/Mesh/HexagonConcentricCircleAdaptiveBoundaryMeshGenerator/sides_to_adapt) and [!param](/Mesh/HexagonConcentricCircleAdaptiveBoundaryMeshGenerator/meshes_to_adapt_to) to generate [PolygonConcentricCircleMeshGenerator.md] -style hexagonal mesh with assembly mesh metadata. The azimuthal discretization of the sides listed in [!param](/Mesh/HexagonConcentricCircleAdaptiveBoundaryMeshGenerator/sides_to_adapt) will be identical to the input meshes [!param](/Mesh/HexagonConcentricCircleAdaptiveBoundaryMeshGenerator/meshes_to_adapt_to) and overwrite the appropriate entry(s) in [!param](/Mesh/HexagonConcentricCircleAdaptiveBoundaryMeshGenerator/num_sectors_per_side).
- The size of the hexagon ([!param](/Mesh/HexagonConcentricCircleAdaptiveBoundaryMeshGenerator/hexagon_size)) must match the size of the input meshes. Default style for providing [!param](/Mesh/HexagonConcentricCircleAdaptiveBoundaryMeshGenerator/hexagon_size) is `apothem` (half-pitch).

### Example

!media tutorial04_meshing/base_ex_hccabmg.png
       id=tutorial04-base_ex_hccabmg
       caption=A schematic drawing showing the input and generated meshes of [HexagonConcentricCircleAdaptiveBoundaryMeshGenerator.md].
       style=width:50%;display:block;margin-left:auto;margin-right:auto;

!listing base_mesh_generators/common_geo.i
         id=tutorial04-hex_2d.i
         caption= Reflector-style assembly example input.
         block=Mesh/adaptive_assm

## Core

The highest level of reactor geometry is generally the core, which consists of multiple assemblies patterned together. [PatternedHexMeshGenerator.md] can be used to create cores similar to how it is used to create assemblies. The main difference is that the inputs to a core object are assemblies rather than pins, and the parameter [!param](/Mesh/PatternedHexMeshGenerator/generate_core_metadata) should be set to `true` when creating a core. This tells MOOSE that the generated object is a core rather than an assembly, which is required to identify eligible mesh operations after creation (for example trimming along a line).

### Object

- [PatternedHexMeshGenerator.md]
- (Cartesian sibling -- [PatternedCartesianMeshGenerator.md])

### Geometry Features

- 2D regular assembly pattern with uniform assembly pitch

### Notes

- Assembly meshes generated by [PatternedHexMeshGenerator.md], [HexagonConcentricCircleAdaptiveBoundaryMeshGenerator.md] and [SimpleHexagonGenerator.md] can be used as inputs. If [SimpleHexagonGenerator.md] is used, the assemblies are homogenized.
- Core mesh generation should include the parameter [!param](/Mesh/PatternedHexMeshGenerator/generate_core_metadata) as `true`.
- The input pattern will be automatically rotated 90 degrees counterclockwise into a "vertex up" position unless otherwise specified with [!param](/Mesh/PatternedHexMeshGenerator/rotate_angle)

### Example

!media tutorial04_meshing/base_ex_phmg_core.png
       id=tutorial04-base_ex_phmg_core
       caption=A example showing a core mesh generated by [PatternedHexMeshGenerator.md] with six identical assembly meshes and one reflector mesh stitched together.
       style=width:50%;display:block;margin-left:auto;margin-right:auto;

!listing base_mesh_generators/common_geo.i
         id=tutorial04-common_geo_pattern_core
         caption=Core assembly example input.
         block=Mesh/pattern_core

## Core Periphery

Nuclear reactor cores are often surrounded by cylindrical reflector barrels to prevent neutron leakages and improve shielding. Once the 2D core is created, a circular reflector, called the "core periphery" in the Reactor module, can be added to the outer ring of assemblies. Two options are available for meshing the core periphery: quadrilateral elements ([PeripheralRingMeshGenerator.md]) or triangular elements ([PeripheralTriangleMeshGenerator.md]).

### Object

- [PeripheralRingMeshGenerator.md] (abbreviated as PRMG)
- [PeripheralTriangleMeshGenerator.md] (abbreviated as PTMG)

### Geometry Features

- Adds circular peripheral region to a reactor core mesh
- [PeripheralRingMeshGenerator.md] has a boundary layer capability

### Notes

- [PeripheralRingMeshGenerator.md] generates a structured peripheral mesh with +QUAD4+ elements
- [PeripheralTriangleMeshGenerator.md] generates an unstructured peripheral mesh with +TRI3+ elements

### PRMG Example

!media tutorial04_meshing/base_ex_prmg.png
       id=tutorial04-base_ex_prmg
       caption=Peripheral ring mesh generated by [PeripheralRingMeshGenerator.md].
       style=width:50%;display:block;margin-left:auto;margin-right:auto;

!listing base_mesh_generators/common_geo.i
         id=tutorial04-core_ring.i
         caption=PRMG example input.
         block=Mesh/pr

### PTMG Example

!media tutorial04_meshing/base_ex_ptmg.png
       id=tutorial04-base_ex_ptmg
       caption=Peripheral triangle mesh generated by [PeripheralTriangleMeshGenerator.md].
       style=width:50%;display:block;margin-left:auto;margin-right:auto;

!listing base_mesh_generators/common_geo.i
         id=tutorial04-core_triangle.i
         caption=PTMG example input.
         block=Mesh/pt

!content pagination previous=tutorial04_meshing/step04_terminology.md
                    next=tutorial04_meshing/step06_common_ops.md
