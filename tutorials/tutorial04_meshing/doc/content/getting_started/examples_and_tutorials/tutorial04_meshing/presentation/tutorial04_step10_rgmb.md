# Reactor Geometry Mesh Builder: A Contained System for Building Regular Geometries

!---

Reactor Geometry Mesh Builder (RGMB) refers to a subset of specialized mesh generators designed specifically for simplifying the task of building reactor geometries. These mesh generators are intended to be called in a specific order and assume that pin-like structures are built into a Cartesian or hexagonal assembly lattice. These assembly lattices can be combined into a core lattice with a peripheral ring added to the core boundary. RGMB mesh generation is accomplished by calling base Reactor module mesh generators under-the-hood while exposing only the minimum number of parameters needed by the user to define the reactor geometry.

!---

Benefits of RGMB system include:

- Simplified user options for generation of reactor meshes
- Minimal number of blocks associated with output mesh -- one block is allocated to all quadrilateral elements and another block is used for all triangular elements
- Automatic assignment of extra element integers: `pin_id`, `assembly_id`, `plane_id`
- Assignment of region ids directly on the mesh, avoiding the need to define these map materials to mesh in the MOOSE physics application input file

The RGMB system is useful for regular 2D or extruded 3D Cartesian or hexagonal geometries. Many useful meshing options (e.g., boundary layers) are currently hidden and unavailable to the user through RGMB to keep things simple. If these options are needed for more complex geometries, the individual Reactor Module mesh generators should be used as shown in earlier sections. Additionally, control drum support has not yet been added.

!---

## ReactorMeshParams

[ReactorMeshParams.md] acts as a container for storing global data about the reactor geometry that needs to be retrieved at different stages of the RGMB mesh generation workflow. In particular, the union axial grid for the extruded geometry is defined here and propagated to the entire mesh upon the extrusion step.

!listing rgmb_mesh_generators/rgmb_core_cartesian.i
         block=Mesh/rmp
         link=False

!---

## PinMeshGenerator

[PinMeshGenerator.md] calls [PolygonConcentricCircleMeshGenerator.md] to generate a Cartesian or hexagonal pin-like structure (pin, background, and duct)

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
       style=width:100%;display:block;margin-left:auto;margin-right:auto;

!row-end!

!---

## AssemblyMeshGenerator

[AssemblyMeshGenerator.md] calls [PatternedHexMeshGenerator.md] or [PatternedMeshGenerator.md] to generate a Cartesian or hexagonal lattice of pin-like structures.

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

[CoreMeshGenerator.md] calls [PatternedHexMeshGenerator.md] or [PatternedMeshGenerator.md] to generate a Cartesian or hexagonal lattice of assembly-like structures.

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
