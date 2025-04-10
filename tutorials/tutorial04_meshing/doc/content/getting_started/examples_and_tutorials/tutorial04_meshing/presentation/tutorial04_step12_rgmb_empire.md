# RGMB Example: 2-D Empire Microreactor Core Mesh with Control Drum Stitching

!---

## 2-D Empire Microreactor Core with Control Drum Stitching

This example illustrates the use of RGMB mesh generators to define a heterogeneous 2D hexagonal microreactor core with control drum structures for the EMPIRE problem ([!cite](matthews2021coupled)). The primary focus of this tutorial is to familiarize users with [ControlDrumMeshGenerator.md] to create control drum structures for stitching into the core lattice.

!media tutorial04_meshing/rgmb_empire_stepbystep.png
       style=width:100%;display:block;margin-left:auto;margin-right:auto;

+Hands-on package MOOSE input file (Heterogeneous EMPIRE mesh)+: `combined/reactor_workshop/tests/reactor_examples/rgmb_empire/rgmb_empire.i`

!---

## ReactorMeshParams

[ReactorMeshParams.md] contains global mesh/geometry parameters including whether the final mesh is 2D or 3D, Cartesian or hexagonal, assembly pitch, and the axial discretization for the final extruded geometry. Here we also invoke the option to enable flexible assembly stitching with [!param](/Mesh/ReactorMeshParams/flexible_assembly_stitching), so that dissimilar assembly structures can be stitched into the reactor core without any hanging nodes. This parameter needs to be set to true in order to use [ControlDrumMeshGenerator.md].

!listing reactor_examples/rgmb_empire/rgmb_empire.i
         block=Mesh/rmp
         link=False

!---

## Pin structures using PinMeshGenerator

The EMPIRE core features heterogeneous fuel assemblies, homogeneous air and reflector regions, and control drum structures all stitched into a hexagonal core lattice. We begin with defining the heterogeneous fuel assemblies.

[PinMeshGenerator.md] defines the constituent pin structures used for stitching into assemblies. The pin pitch, number of azimuthal sectors, and geometry / region ID information about each ring, background, and duct region are specified here.

[PinMeshGenerator.md] is called multiple times to define the various pin structures (3 fuel pin types, 1 heat pipe pintype, and 1 moderator pin type).

!listing reactor_examples/rgmb_empire/rgmb_empire.i
         block=Mesh/fuel_pin_1
         link=False

!---

## Assembly structures using AssemblyMeshGenerator

[AssemblyMeshGenerator.md] takes the pin types previously defined and places them into a regular hexagonal grid. Additionally, a stainless steel background region is added around the pins in order to create the assembly geometry.

[AssemblyMeshGenerator.md] is called multiple times to define the various heterogeneous assemblies (3 fuel assemblies).

!listing reactor_examples/rgmb_empire/rgmb_empire.i
         block=Mesh/fuel_assembly_1
         link=False

!---

## Homogeneous assembly structures using PinMeshGenerator

In order to define homogeneous assembly structures to stitch into the core, we use [PinMeshGenerator.md] once again ([AssemblyMeshGenerator.md] is only used for structures that consist of lattices of pins).

- +To define single assemblies directly with PinMeshGenerators for stitching with [CoreMeshGenerator.md]+, [PinMeshGenerator.md] is used with [!param](/Mesh/PinMeshGenerator/use_as_assembly) set to `true`. In addition, [!param](/Mesh/PinMeshGenerator/homogenized) = `true` is used to indicate that this region is homogenized.

!listing reactor_examples/rgmb_empire/rgmb_empire.i
         block=Mesh/refl_assembly
         link=False

!---

## Control drum structures using ControlDrumMeshGenerator

In order to define control drum structures to stitch into the core, we use [ControlDrumMeshGenerator.md].

- [!param](/Mesh/ControlDrumMeshGenerator/drum_inner_radius) and [!param](/Mesh/ControlDrumMeshGenerator/drum_outer_radius) control the inner and outer radius of the drum region, while [!param](/Mesh/ControlDrumMeshGenerator/drum_inner_intervals) and [!param](/Mesh/ControlDrumMeshGenerator/drum_intervals) control the radial mesh density of the drum inner and drum regions respectively.
- [!param](/Mesh/ControlDrumMeshGenerator/num_azimuthal_sectors) controls the azimuthal mesh density of the control drum structure.
- Additional parameters are provided to [ControlDrumMeshGenerator.md] to create explicit drum pad regions. [!param](/Mesh/ControlDrumMeshGenerator/pad_start_angle) and [!param](/Mesh/ControlDrumMeshGenerator/pad_end_angle) set the start and end angles of the drum pad region.
- The [!param](/Mesh/ControlDrumMeshGenerator/region_ids) parameter takes four values per axial layer, representing the four radial regions of the control drum structure (drum inner, drum pad, drum ex-pad, background)

This process is repeated for each of the 6 unique drum orientations that are part of the core lattice.

!listing reactor_examples/rgmb_empire/rgmb_empire.i
         block=Mesh/cd_ne
         link=False

!---


## Heterogeneous core using CoreMeshGenerator

[CoreMeshGenerator.md] takes the input assemblies (heterogeneous and homogeneous) and drum structures and stitches them into a core.

- Since [!param](/Mesh/ReactorMeshParams/flexible_assembly_stitching) = `true` in [ReactorMeshParams.md], the different assembly types are stitched together without hanging nodes.

!listing reactor_examples/rgmb_empire/rgmb_empire.i
         block=Mesh/core
         link=False

!media tutorial04_meshing/rgmb_empire_mesh.png
       style=width:100%;display:block;margin-left:auto;margin-right:auto;

!---

## Use of RGMB Mesh with Griffin

Griffin recognizes material ID assignments through the `material_id` tag. Therefore, the `region_id` tags need to be renamed to `material_id`. This is done using [ExtraElementIDCopyGenerator.md].

!listing reactor_examples/rgmb_empire/rgmb_empire.i
         block=Mesh/empire_mesh
         link=False

Material definition in the Griffin input file is then greatly simplified since `material_id` is defined directly on mesh. No additional mapping is needed.

!listing reactor_examples/rgmb_empire/rgmb_empire_griffin_snippet.i
         block=Materials
         link=False

!---

RGMB labels outer boundary sidesets with pre-defined names -- "top" for top boundary, "bottom" for bottom boundary, and "outer_core" for radial boundary. Boundary conditions are assigned to these sidesets in Griffin. Since this is a 2-D mesh, only the "outer_core" boundary sideset is created by RGMB.

!listing reactor_examples/rgmb_empire/rgmb_empire_griffin_snippet.i
         block=TransportSystems
         link=False
