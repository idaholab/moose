# Reactor Geometry Mesh Builder Example: 2-D Empire Microreactor Core Mesh with Control Drum Stitching

This example illustrates the use of RGMB mesh generators to define a heterogeneous 2D hexagonal microreactor core with control drum structures for the EMPIRE problem ([!cite](matthews2021coupled)). The primary focus of this tutorial is to familiarize users with [ControlDrumMeshGenerator.md] to create control drum structures for stitching into the core lattice. For more information about RGMB and how to use it for a conventional fast reactor problem, please see [step10_rgmb.md] and [step11_rgmb_het_hom.md], respectively.

!media tutorial04_meshing/rgmb_empire_stepbystep.png
       id=tutorial04-rgmb_abtr_stepbystep
       caption=Visualization of meshing steps to build the 2D EMPIRE core mesh with RGMB.
       style=width:75%;display:block;margin-left:auto;margin-right:auto;

## ReactorMeshParams

[ReactorMeshParams.md] contains global mesh/geometry parameters including whether the final mesh is 2D or 3D, Cartesian or hexagonal, assembly pitch, and the axial discretization for the final extruded geometry. This information will be accessible to the other RGMB mesh generators and consistently used. Here we also invoke the option to enable flexible assembly stitching with [!param](/Mesh/ReactorMeshParams/flexible_assembly_stitching), so that dissimilar assembly structures can be stitched into the reactor core without any hanging nodes. This parameter needs to be set to true in order to use [ControlDrumMeshGenerator.md].

!listing reactor_examples/rgmb_empire/rgmb_empire.i
         id=tutorial04-rgmb_empire-rmp
         caption=EMPIRE RGMB Reactor Mesh Parameters example.
         block=Mesh/rmp

## Pin structures using PinMeshGenerator

The EMPIRE core features heterogeneous fuel assemblies, homogeneous air and reflector regions, and control drum structures all stitched into a hexagonal core lattice. We begin with defining the heterogeneous fuel assemblies.

[PinMeshGenerator.md] defines the constituent pin structures used for stitching into the fuel assemblies. The pin pitch, number of azimuthal sectors, and geometry / region ID information about each ring, background, and duct region are specified here.

[PinMeshGenerator.md] is called multiple times to define the various pin structures (3 fuel pin types, 1 heat pipe pin type, and 1 moderator pin type).

!listing reactor_examples/rgmb_empire/rgmb_empire.i
         id=tutorial04-rgmb_empire-pin
         caption=EMPIRE RGMB pin example.
         block=Mesh/fuel_pin_1

!alert! note title=Tips

- [!param](/Mesh/PinMeshGenerator/region_ids) is a 2-dimensional array containing region IDs (essentially materials). Since this problem is 2-D and does not have any axial layers, we only need to define a single row of values corresponding to the 2D radial regions (from center of the pin to outermost region) of the pin. In this case, the fuel pin has 3 radial regions (fuel inner ring, gap outer ring, statinless steel background).

!alert-end!

## Assembly structures using AssemblyMeshGenerator

[AssemblyMeshGenerator.md] takes the pin types previously defined and places them into a regular hexagonal grid. Additionally, a stainless steel background region is added around the pins in order to create the assembly geometry.

[AssemblyMeshGenerator.md] is called multiple times to define the various heterogeneous assemblies (3 fuel assemblies).

!listing reactor_examples/rgmb_empire/rgmb_empire.i
         id=tutorial04-rgmb_empire-het-assembly
         caption=EMPIRE RGMB heterogeneous assembly example.
         block=Mesh/fuel_assembly_1

!alert! note title=Tips

- Use a unique [AssemblyMeshGenerator.md] block for each assembly with a unique geometrical configuration, region ID composition, and/or inventory of pin structures
- [!param](/Mesh/AssemblyMeshGenerator/background_region_id) is a 1-dimensional array containing region IDs for each axial layer of the background region (since this is a 2-D mesh, only a single value needs to be provided).

!alert-end!

## Homogeneous assembly structures using PinMeshGenerator

In order to define homogeneous assembly structures to stitch into the core, we use [PinMeshGenerator.md] once again ([AssemblyMeshGenerator.md] is only used for structures that consist of lattices of pins).

- +To define single assemblies directly with PinMeshGenerators for stitching with [CoreMeshGenerator.md]+, [PinMeshGenerator.md] is used with [!param](/Mesh/PinMeshGenerator/use_as_assembly) set to `true`.
- In addition, [!param](/Mesh/PinMeshGenerator/homogenized) = `true` is used to indicate that this region is homogenized.

This process is repeated for each homogeneous assembly type (1 airhole assembly and 1 reflector assembly).

!listing reactor_examples/rgmb_empire/rgmb_empire.i
         id=tutorial04-rgmb_empire-hom-assembly
         caption=EMPIRE RGMB homogeneous assembly example.
         block=Mesh/refl_assembly

!alert! note title=Tips

- For homogenized assemblies, each assembly has only 1 radial region, so a single value is passed to [!param](/Mesh/PinMeshGenerator/region_ids) representing the region ID of the 2-D homogenized region.

!alert-end!

## Control drum structures using ControlDrumMeshGenerator

In order to define control drum structures to stitch into the core, we use [ControlDrumMeshGenerator.md]. [!param](/Mesh/ControlDrumMeshGenerator/drum_inner_radius) and [!param](/Mesh/ControlDrumMeshGenerator/drum_outer_radius) control the inner and outer radius of the drum region, while [!param](/Mesh/ControlDrumMeshGenerator/drum_inner_intervals) and [!param](/Mesh/ControlDrumMeshGenerator/drum_intervals) control the radial mesh density of the drum inner and drum regions respectively. [!param](/Mesh/ControlDrumMeshGenerator/num_azimuthal_sectors) controls the azimuthal mesh density of the control drum structure.

Since we are interested in definining an explicit drum region for this structure, additional parameters are provided to [ControlDrumMeshGenerator.md] to specify this. [!param](/Mesh/ControlDrumMeshGenerator/pad_start_angle) and [!param](/Mesh/ControlDrumMeshGenerator/pad_end_angle) set the start and end angles of the drum pad region, where angles start in the positive y direction and rotate clockwise to indicate positive angles. Additionally, the [!param](/Mesh/ControlDrumMeshGenerator/region_ids) parameter takes four values per axial layer, representing the four radial regions of the control drum structure (drum inner, drum pad, drum ex-pad, background)

This process is repeated for each control drum type. In this case, each unique control drum orientation is considered a separate type. Since we are generating 12 control drums with a total of 6 unique drum orientations, we create six control drum objects.

!listing reactor_examples/rgmb_empire/rgmb_empire.i
         id=tutorial04-rgmb_empire-control-drum
         caption=EMPIRE RGMB control drum example.
         block=Mesh/cd_ne

## Heterogeneous core using CoreMeshGenerator

Now that all heterogeneous and homogeneous assemblies and control drum structures have been defined, they are placed into a lattice using [CoreMeshGenerator.md]. Since we have set [!param](/Mesh/ReactorMeshParams/flexible_assembly_stitching) = `true` in [ReactorMeshParams.md], RGMB will automatically take care of stitching these assembly types together by making sure the same number of nodes are defined on each interface of each assembly. 

!listing reactor_examples/rgmb_empire/rgmb_empire.i
         id=tutorial04-rgmb_empire-het_core
         caption=EMPIRE RGMB core example.
         block=Mesh/core

!media tutorial04_meshing/rgmb_empire_mesh.png
       id=tutorial04-rgmb_empire_core
       caption=RGMB-generated 2D EMPIRE core mesh, colored by region ID.
       style=width:75%;display:block;margin-left:auto;margin-right:auto;

## Use of RGMB Mesh with Griffin

Griffin recognizes material ID assignments through the `material_id` tag. Therefore, the `region_id` tags need to be renamed to `material_id`. This is done using [ExtraElementIDCopyGenerator.md].

!listing reactor_examples/rgmb_empire/rgmb_empire.i
         id=tutorial04-rgmb_empire-matid
         caption=EMPIRE material ID setup example.
         block=Mesh/empire_mesh

Material definition in the Griffin input file is then greatly simplified since `material_id` is defined directly on mesh. No additional mapping is needed.

!listing reactor_examples/rgmb_empire/rgmb_empire_griffin_snippet.i
         id=tutorial04-empire_griffin_materials
         caption=Griffin materials setup.
         block=Materials

RGMB labels outer boundary sidesets with pre-defined names -- "top" for top boundary, "bottom" for bottom boundary, and "outer_core" for radial boundary. Boundary conditions are assigned to these sidesets in Griffin. Since this is a 2-D mesh, only the "outer_core" boundary sideset is created by RGMB.

!listing reactor_examples/rgmb_empire/rgmb_empire_griffin_snippet.i
         id=tutorial04-empire_griffin_bcs
         caption=Griffin Boundary conditions setup.
         block=TransportSystems

!bibtex bibliography !!include to make sure next/previous are last on page

!content pagination previous=tutorial04_meshing/step11_rgmb_het_hom.md
                    next=tutorial04_meshing/step13_advanced_tools.md
