# Reactor Geometry Mesh Builder: A Contained System for Building Regular Geometries

Reactor Geometry Mesh Builder (RGMB) refers to a subset of specialized mesh generators designed specifically for simplifying the task of building reactor geometries. These mesh generators are intended to be called in a specific order and assume that pin-like structures are built into a Cartesian or hexagonal assembly lattice. These assembly lattices can be combined into a core lattice with a peripheral ring added to the core boundary. RGMB mesh generation is accomplished by calling base Reactor module mesh generators under-the-hood while exposing only the minimum number of parameters needed by the user to define the reactor geometry.

Benefits of RGMB system include:

- Simplified user options for generation of reactor meshes
- Minimal number of blocks associated with output mesh -- one block is allocated to all quadrilateral elements and another block is used for all triangular elements
- Automatic assignment of extra element integers: `pin_id`, `assembly_id`, `plane_id`
- Assignment of region ids directly on the mesh, avoiding the need to define these map materials to mesh in the MOOSE physics application input file

The RGMB system is useful for regular 2D or extruded 3D Cartesian or hexagonal geometries. Many useful meshing options (e.g., boundary layers) are currently hidden and unavailable to the user through RGMB to keep things simple. If these options are needed for more complex geometries, the individual Reactor Module mesh generators should be used as shown in earlier sections. Additionally, control drum support has not yet been added.

RGMB consists of a few specific mesh generators which are to be called in order and detailed next.

## ReactorMeshParams

[ReactorMeshParams.md] acts as a container for storing global data about the reactor geometry that needs to be retrieved at different stages of the RGMB mesh generation workflow. In particular, the union axial grid for the extruded geometry is defined here and propagated to the entire mesh upon the extrusion step.

!listing rgmb_mesh_generators/rgmb_core_cartesian.i
         id=tutorial04-rgmb_core_cartesian-rmp
         caption=Reactor Mesh Parameters example.
         block=Mesh/rmp

## PinMeshGenerator

[PinMeshGenerator.md] calls [PolygonConcentricCircleMeshGenerator.md] to generate a Cartesian or hexagonal pin-like structure (pin, background, and duct)

!listing rgmb_mesh_generators/rgmb_core_cartesian.i
         id=tutorial04-rgmb_core_cartesian-pin1
         caption=RGMB Pin example.
         block=Mesh/pin1

The final mesh will contain one block (subdomain) for quadrilateral elements and one block for triangular elements. Regions with different materials are distinguished by the `region_id` reporting ID rather than by subdomain ID.

[!param](/Mesh/PinMeshGenerator/region_ids) is a 2D array parameter that sets the `region_id` by radial region (rows) and axial region (column). The 3D material map is stored for use after extrusion, which could happen at the pin, assembly, or core step, whichever step is final for the problem of interest.

!media tutorial04_meshing/rgmb_pinmesh_cart.png
       id=tutorial04-rgmb_pinmesh_cart
       caption=Example RGMB Cartesian pin cell mesh colored by `subdomain_id` (left) and `region_id` (right).
       style=width:50%;display:block;margin-left:auto;margin-right:auto;

!listing rgmb_mesh_generators/rgmb_core_cartesian.i
         id=tutorial04-rgmb_pinmesh_cart-pin1
         caption=Example RGMB Cartesian pin cell.
         block=Mesh/pin1

!media tutorial04_meshing/rgmb_pinmesh_hex.png
       id=tutorial04-rgmb_pinmesh_hex
       caption=Example RGMB hexagonal pin cell mesh colored by `subdomain_id` (left) and `region_id` (right).
       style=width:50%;display:block;margin-left:auto;margin-right:auto;

!listing rgmb_mesh_generators/rgmb_core_hexagonal.i
         id=tutorial04-rgmb_pinmesh_hex-pin1
         caption=Example RGMB hexagonal pin cell.
         block=Mesh/pin1

## AssemblyMeshGenerator

[AssemblyMeshGenerator.md] calls [PatternedHexMeshGenerator.md] or [PatternedMeshGenerator.md] to generate a Cartesian or hexagonal lattice of pin-like structures. Cartesian and hexagonal examples follow.

### RGMB Cartesian Assembly Example

- Tips:

  - Elements belonging to a unique pin cell will be automatically provided a unique `pin_id`
  - Ducts for Cartesian assemblies are not currently supported

!media tutorial04_meshing/rgmb_assemblymesh_cart_blockid.png
       id=tutorial04-rgmb_assemblymesh_cart_blockid
       caption=Cartesian assembly colored by `subdomain_id` (left), `region_id` (middle), and `pin_id` (right). Subdomain ids categorize elements as belonging to different subdomains (blocks), region ids categorize elements belonging to different regions (material zones), and pin ids categorize elements belonging to different pin cells.
       style=width:75%;display:block;margin-left:auto;margin-right:auto;

!listing rgmb_mesh_generators/rgmb_core_cartesian.i
         id=tutorial04-rgmb_rgmb_core_cartesian
         caption=Example RGMB Cartesian assembly.
         block=Mesh/assembly1

### RGMB Hexagonal Assembly Example

[AssemblyMeshGenerator.md] calls [PatternedHexMeshGenerator.md] or [PatternedMeshGenerator.md] to generate a Cartesian or hexagonal lattice of pin-like structures.

- Tips:

  - Hexagonal ducts and background coolant region can be added to hexagonal assemblies. The region IDs for the duct and background regions are set by the `duct_region_ids` and `background_region_id` parameters.
  - Don't want explicitly defined pins in your assembly? See [step11_rgmb_homo.md] for how to define homogenized assemblies with [AssemblyMeshGenerator.md].

!media tutorial04_meshing/rgmb_assemblymesh_hex.png
       id=tutorial04-rgmb_assemblymesh_hex
       caption=Hexagonal assembly colored by `subdomain_id` (left), `region_id` (middle), and `pin_id` (right). Subdomain ids categorize elements as belonging to different subdomains (blocks), region ids categorize elements belonging to different regions (material zones), and pin ids categorize elements belonging to different pin cells.
       style=width:75%;display:block;margin-left:auto;margin-right:auto;

!listing rgmb_mesh_generators/rgmb_core_hexagonal.i
         id=tutorial04-rgmb_rgmb_core_hexagonal
         caption=Example RGMB hexagonal assembly.
         block=Mesh/assembly1

## CoreMeshGenerator

[CoreMeshGenerator.md] calls [PatternedHexMeshGenerator.md] or [PatternedMeshGenerator.md] to generate a Cartesian or hexagonal lattice of assembly-like structures.

Defining [!param](/Mesh/CoreMeshGenerator/dummy_assembly_name) and using it in inputs defines a dummy assembly in the core.

### Cartesian Core Example

!media tutorial04_meshing/rgmb_coremesh_cart.png
       id=tutorial04-rgmb_coremesh_cart
       caption=Cartesian core colored by `subdomain_id` (left), `region_id` (middle), and `assembly_id` (right).
       style=width:75%;display:block;margin-left:auto;margin-right:auto;

!listing rgmb_mesh_generators/rgmb_core_cartesian.i
         id=tutorial04-rgmb_core_cart-core
         caption=Example RGMB Cartesian core.
         block=Mesh/rgmb_core

### Hexagonal Core Example

!media tutorial04_meshing/rgmb_coremesh_hex.png
       id=tutorial04-rgmb_coremesh_hex
       caption=Hexagonal core colored by `subdomain_id` (left), `region_id` (middle), and `assembly_id` (right).
       style=width:75%;display:block;margin-left:auto;margin-right:auto;

!listing rgmb_mesh_generators/rgmb_core_hexagonal.i
         id=tutorial04-rgmb_core_hex-core
         caption=Example RGMB Hexagonal core.
         block=Mesh/rgmb_core

## CoreMeshGenerator with Peripheral Ring

A core periphery region can be added utilizing either the [PeripheralRingMeshGenerator.md] or the [PeripheralTriangleMeshGenerator.md].

!media tutorial04_meshing/rgmb_periphery.png
       id=tutorial04-rgmb_periphery
       caption=Hexagonal core with core periphery meshed.
       style=width:50%;display:block;margin-left:auto;margin-right:auto;

!listing rgmb_mesh_generators/rgmb_core_hexagonal_periphery.i
         id=tutorial04-rgmb_core_hex-periphery
         caption=Example RGMB Hexagonal core with periphery.
         block=Mesh/rgmb_core

## Default Block and External Boundary Names

An RGMB case can terminate at the pin, assembly, or core "level". Pins and assemblies always have a "type" assigned (a numeric integer specified as [!param](/Mesh/PinMeshGenerator/pin_type) for pins and [!param](/Mesh/AssemblyMeshGenerator/assembly_type) for assemblies), whereas cores do not. The "level" and "type" are used in the naming schemes for the blocks and outer boundary as follows:

- The default block names in the final mesh are `RGMB_(level)(type)` and `RGMB_(level)(type)_TRI` (if triangular elements are present). For example, `RGMB_PIN1` and `RMGB_PIN1_TRI`, or `RGMB_ASSEMBLY1` and `RGMB_ASSEMBLY1_TRI`, or `RGMB_CORE` and `RGMB_CORE_TRI`, for workflows ending at the pin, assembly or core level, respectively.
- The default outer boundary names in the radial dimension are `outer_(level)_(type)`. For example, for workflows ending at the pin, assembly or core level respectively, the outer boundary names are `outer_pin_1`, `outer_assembly_1` and `outer_core`.
- If the problem is extruded in the axial dimension, the `top` and `bottom` boundaries also exist.

!content pagination previous=tutorial04_meshing/step09_hpmr.md
                    next=tutorial04_meshing/step11_rgmb_homo.md
