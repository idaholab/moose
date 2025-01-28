# Reactor Geometry Mesh Builder (RGMB): A Contained System for Building Regular Geometries

!---

Reactor Geometry Mesh Builder (RGMB) refers to a subset of specialized mesh generators designed specifically for simplifying the task of building reactor geometries. These mesh generators are intended to be called in a specific order and assume that pin-like structures are built into a Cartesian or hexagonal assembly lattice. These assembly lattices can be combined into a core lattice with a peripheral ring added to the core boundary. RGMB mesh generation is accomplished by calling base Reactor module mesh generators under-the-hood while exposing only the minimum number of parameters needed by the user to define the reactor geometry.

!---

Benefits of RGMB system include:

- Simplified user options for generation of reactor meshes
- Minimal number of blocks associated with output mesh -- one block is allocated to all quadrilateral elements and another block is used for all triangular elements
- Automatic assignment of extra element integers: `pin_id`, `assembly_id`, `plane_id`
- Assignment of region ids directly on the mesh, avoiding the need to define these map materials to mesh in the MOOSE physics application input file

The RGMB system is useful for regular 2D or extruded 3D Cartesian or hexagonal geometries. Many useful meshing options (e.g., boundary layers) are currently hidden and unavailable to the user through RGMB to keep things simple. If these options are needed for more complex geometries, the individual Reactor Module mesh generators should be used as shown in earlier sections. Additionally, control drum mesh generation support has been added for hexagonal core layouts.

!---

## ReactorMeshParams

[ReactorMeshParams.md] acts as a container for storing global data about the reactor geometry that needs to be retrieved at different stages of the RGMB mesh generation workflow. In particular, the union axial grid for the extruded geometry is defined here and propagated to the entire mesh upon the extrusion step.

!listing rgmb_mesh_generators/rgmb_core_cartesian.i
         block=Mesh/rmp
         link=False

!---

## PinMeshGenerator

[PinMeshGenerator.md] calls [PolygonConcentricCircleMeshGenerator.md] to generate a Cartesian or hexagonal pin-like structure (pin, background, and duct) (mesh colored by `subdomain_id` (left) and `region_id` (right))

!row!
!col small=12 medium=6 large=8

!listing rgmb_mesh_generators/rgmb_core_cartesian.i
         block=Mesh/pin1
         link=False

!col small=12 medium=6 large=4

!media tutorial04_meshing/rgmb_pinmesh_cart.png
       style=width:100%;display:block;margin-left:auto;margin-right:auto;

!row-end!

!row!
!col small=12 medium=6 large=8

!listing rgmb_mesh_generators/rgmb_core_hexagonal.i
         block=Mesh/pin1
         link=False

!col small=12 medium=6 large=4

!media tutorial04_meshing/rgmb_pinmesh_hex.png
       style=width:70%;display:block;margin-left:auto;margin-right:auto;

!row-end!

!---

## AssemblyMeshGenerator

[AssemblyMeshGenerator.md] calls [PatternedHexMeshGenerator.md] or [PatternedMeshGenerator.md] to generate a Cartesian or hexagonal lattice of pin-like structures. (assembly colored by `subdomain_id` (left), `region_id` (middle), and `pin_id` (right))

!row!
!col small=12 medium=6 large=8

!listing rgmb_mesh_generators/rgmb_core_cartesian.i
         block=Mesh/assembly1
         link=False

!col small=12 medium=6 large=4

!media tutorial04_meshing/rgmb_assemblymesh_cart_blockid.png
       style=width:100%;display:block;margin-left:auto;margin-right:auto;

!row-end!

!row!
!col small=12 medium=6 large=8

!listing rgmb_mesh_generators/rgmb_core_hexagonal.i
         block=Mesh/assembly1
         link=False

!col small=12 medium=6 large=4

!media tutorial04_meshing/rgmb_assemblymesh_hex.png
       style=width:100%;display:block;margin-left:auto;margin-right:auto;

!row-end!

!---

## CoreMeshGenerator

[CoreMeshGenerator.md] calls [PatternedHexMeshGenerator.md] or [PatternedMeshGenerator.md] to generate a Cartesian or hexagonal lattice of assembly-like structures. (core colored by `subdomain_id` (left), `region_id` (middle), and `assembly_id` (right))

!row!
!col small=12 medium=6 large=8

!listing rgmb_mesh_generators/rgmb_core_cartesian.i
         block=Mesh/rgmb_core
         link=False

!col small=12 medium=6 large=4

!media tutorial04_meshing/rgmb_coremesh_cart.png
       style=width:100%;display:block;margin-left:auto;margin-right:auto;

!row-end!

!row!
!col small=12 medium=6 large=8

!listing rgmb_mesh_generators/rgmb_core_hexagonal.i
         block=Mesh/rgmb_core
         link=False

!col small=12 medium=6 large=4

!media tutorial04_meshing/rgmb_coremesh_hex.png
       style=width:100%;display:block;margin-left:auto;margin-right:auto;

!row-end!

!---

## Flexible Assembly Stitching in RGMB

By default, [CoreMeshGenerator.md] does not consider the location of nodes at the boundary of assemblies when stitching the core lattice. For dissimilar assemblies, this can lead to hanging nodes at the assembly interface. The following scenarios can cause hanging nodes in the output core mesh:

!row!
!col width=50%
1: Two assemblies have the same constituent pin geometry but vary in total number of pins in the pin lattice

!col width=50%
!media reactor/meshgenerators/rgmb_flexible_stitching_case1.png style=width:100%;display:block;margin-left:auto;margin-right:auto;

!row-end!

!row!
!col width=50%
2: Two assemblies have the same pin lattice structure and geometry, but the constituent pins of each assembly are subdivided into a different number of sectors per side.

!col width=50%
!media reactor/meshgenerators/rgmb_flexible_stitching_case2.png style=width:100%;display:block;margin-left:auto;margin-right:auto;

!row-end!

!row!
!col width=50%
3: One assembly is defined as a heterogeneous mesh (contains one or more pins), and the other assembly is homogenized.

!col width=50%
!media reactor/meshgenerators/rgmb_flexible_stitching_case3.png style=width:100%;

!row-end!

The parameter [!param](/Mesh/ReactorMeshParams/flexible_assembly_stitching) in [ReactorMeshParams.md] can be set to true to enable flexible assembly stitching, where the outermost radial layer is deleted and re-triangulated in order to ensure the same number of nodes on either side of the assembly interface. The optional parameter [!param](/Mesh/ReactorMeshParams/num_sectors_at_flexible_boundary) defines how many sectors should be defined at the assembly interface.

!---

## ControlDrumMeshGenerator

[ControlDrumMeshGenerator.md] calls [AdvancedConcentricCircleGenerator.md] and [FlexiblePatternGenerator.md] to generate control drum structures that can be stitched directly into a hexagonal core lattice. This mesh generator supports automatic region ID assignments as well as creation of an explicit drum pad region if desired.

!row!
!col small=12 medium=6 large=8

!listing rgmb_mesh_generators/rgmb_core_cd.i
         block=Mesh/drum_nopad
         link=False

!col small=12 medium=6 large=4

!media tutorial04_meshing/rgmb_drummesh_nopad.png
       style=width:80%;display:block;margin-left:auto;margin-right:auto;

!row-end!

!row!
!col small=12 medium=6 large=8

!listing rgmb_mesh_generators/rgmb_core_cd.i
         block=Mesh/drum_pad
         link=False

!col small=12 medium=6 large=4

!media tutorial04_meshing/rgmb_drummesh_pad.png
       style=width:80%;display:block;margin-left:auto;margin-right:auto;

!row-end!

!---

## CoreMeshGenerator with ControlDrumMeshGenerator

Use of [ControlDrumMeshGenerator.md] allows for drum structures to be stitched directly into the lattice defined in [CoreMeshGenerator.md]. Use of flexible assembly stitching ensures that all assembly structures get stitched together without any hanging nodes.

!row!
!col small=12 medium=6 large=8

!listing rgmb_mesh_generators/rgmb_core_cd.i
         block=Mesh/rgmb_core
         link=False

!col small=12 medium=6 large=4

!media tutorial04_meshing/rgmb_coremesh_cd.png
       style=width:100%;display:block;margin-left:auto;margin-right:auto;

!row-end!

!---

## CoreMeshGenerator with Peripheral Ring

A core periphery region can be added utilizing either the [PeripheralRingMeshGenerator.md] or the [PeripheralTriangleMeshGenerator.md].

!row!
!col small=12 medium=6 large=8

!listing rgmb_mesh_generators/rgmb_core_hexagonal_periphery.i
         block=Mesh/rgmb_core
         link=False

!col small=12 medium=6 large=4

!media tutorial04_meshing/rgmb_periphery.png
       style=width:100%;display:block;margin-left:auto;margin-right:auto;

!row-end!

!---

## Default Block and External Boundary Names

An RGMB case can terminate at the pin, assembly, or core "level". Pins and assemblies always have a "type" assigned (a numeric integer specified as [!param](/Mesh/PinMeshGenerator/pin_type) for pins and [!param](/Mesh/AssemblyMeshGenerator/assembly_type) for assemblies), whereas cores do not. The "level" and "type" are used in the naming schemes for the blocks and outer boundary as follows:

- The default block names in the final mesh are `RGMB_(level)(type)` and `RGMB_(level)(type)_TRI` (if triangular elements are present). For example, `RGMB_PIN1` and `RMGB_PIN1_TRI`, or `RGMB_ASSEMBLY1` and `RGMB_ASSEMBLY1_TRI`, or `RGMB_CORE` and `RGMB_CORE_TRI`, for workflows ending at the pin, assembly or core level, respectively.
- The default outer boundary names in the radial dimension are `outer_(level)_(type)`. For example, for workflows ending at the pin, assembly or core level respectively, the outer boundary names are `outer_pin_1`, `outer_assembly_1` and `outer_core`.
- If the problem is extruded in the axial dimension, the `top` and `bottom` boundaries also exist.
- For users that require a block name that is linked to the region ID of the element, "ReactorMeshParams/region_id_as_block_name" can be set to true to accomplish this. In this case, the block name will be set as `RGMB_(level)(type)_REG(region_id)`, where "region_id" refers to the region ID extra element integer the element. For triangular elements, the suffix "_TRI" will also be added.
