# Reactor Geometry Mesh Builder Example: Homogeneous Sodium-Cooled Fast Reactor Core (ABTR)

!---

## Homogeneous Sodium-Cooled Fast Reactor Core (ABTR)

This example illustrates the use of RGMB mesh generators to define a 3D hexagonal geometry core with homogeneous assemblies (ABTR ([!cite](shemon2015abtr)), constructed earlier in this tutorial using base mesh generators).

!media tutorial04_meshing/rgmb_abtr_stepbystep.png
       style=width:70%;display:block;margin-left:auto;margin-right:auto;

+Hands-on package MOOSE input file+: `combined/reactor_workshop/tests/reactor_examples/rgmb_abtr/rgmb_abtr.i`

!---

## ReactorMeshParams

[ReactorMeshParams.md] contains global mesh/geometry parameters including whether the final mesh is 2D or 3D, Cartesian or hexagonal, assembly pitch, and the axial discretization for the final extruded geometry. This information will be accessible to the other RGMB mesh generators and consistently used.

!listing reactor_examples/rgmb_abtr/rgmb_abtr.i
         block=Mesh/rmp
         link=False

!---

## PinMeshGenerator

- This example does not have any pin-level geometry as the assemblies are homogenized. However, we still start with [PinMeshGenerator.md].
- +To define single assemblies directly with PinMeshGenerators for stitching with [CoreMeshGenerator.md]+, [PinMeshGenerator.md] is used with [!param](/Mesh/PinMeshGenerator/use_as_assembly) set to `true`.
- In addition, [!param](/Mesh/PinMeshGenerator/homogenized) = `true` is used to indicate that this region is homogenized and [SimpleHexagonGenerator.md] should be called to discretize the assembly instead of [PolygonConcentricCircleMeshGenerator.md].

[PinMeshGenerator.md] is called multiple times to define the various homogeneous assemblies. Dummy assemblies are not required when building a core using RGMB, so they are not defined in this input.

!listing reactor_examples/rgmb_abtr/rgmb_abtr.i
         block=Mesh/control
         link=False

!---

## CoreMeshGenerator

- [CoreMeshGenerator.md] has some additional intelligence to automatically handle dummy assembly creation and deletion

!row!
!col small=12 medium=6 large=8

!listing reactor_examples/rgmb_abtr/rgmb_abtr.i
         block=Mesh/core
         link=False

!col small=12 medium=6 large=4

!media tutorial04_meshing/rgmb_abtr_core.png
       style=width:100%;display:block;margin-left:auto;margin-right:auto;

!row-end!

!media tutorial04_meshing/rgmb_abtr_core_regionid.png
       style=width:60%;display:block;margin-left:auto;margin-right:auto;

!---

## Use of RGMB Mesh with Griffin

Griffin recognizes material ID assignments through the `material_id` tag. Therefore, the `region_id` tags need to be renamed to `material_id`. This is done using [ExtraElementIDCopyGenerator.md].

!listing reactor_examples/rgmb_abtr/rgmb_abtr.i
         block=Mesh/abtr_mesh
         link=False

Material definition in the Griffin input file is then greatly simplified since `material_id` is defined directly on mesh. No additional mapping is needed.

!listing reactor_examples/rgmb_abtr/rgmb_abtr_griffin_snippet.i
         block=Materials
         link=False

!---

RGMB labels outer boundary sidesets with pre-defined names -- "top" for top boundary, "bottom" for bottom boundary, and "outer_core" for radial boundary. Boundary conditions are assigned to these sidesets in Griffin.

!listing reactor_examples/rgmb_abtr/rgmb_abtr_griffin_snippet.i
         block=TransportSystems
         link=False
