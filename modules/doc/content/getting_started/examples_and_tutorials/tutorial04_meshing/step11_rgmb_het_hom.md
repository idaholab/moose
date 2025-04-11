# Reactor Geometry Mesh Builder Example: Conversion of Heterogeneous to Homogeneous Sodium-Cooled Fast Reactor Core Mesh (ABTR)

This example illustrates the use of RGMB mesh generators to define a pin-resolved heterogeneous 3D hexagonal geometry core for the Advanced Burner Test Reactor (ABTR) [!cite](shemon2015abtr), and using Griffin to convert this mesh to an equivalent homogenized core mesh with automatic geometry construction and region ID assignment. The final mesh constructed will be similar to the one presented earlier in this tutorial using base mesh generators.

!media tutorial04_meshing/rgmb_abtr_hethom_stepbystep.png
       id=tutorial04-rgmb_abtr_stepbystep
       caption=Visualization of meshing steps to build the 3D homogeneous ABTR core with RGMB and Griffin mesh generators.
       style=width:75%;display:block;margin-left:auto;margin-right:auto;

## ReactorMeshParams

[ReactorMeshParams.md] contains global mesh/geometry parameters including whether the final mesh is 2D or 3D, Cartesian or hexagonal, assembly pitch, and the axial discretization for the final extruded geometry. This information will be accessible to the other RGMB mesh generators and consistently used. Here we also invoke the option to enable flexible assembly stitching, so that dissimilar assembly structures can be stitched into the reactor core without any hanging nodes.

!listing reactor_examples/rgmb_abtr/rgmb_abtr_het_mesh.i
         id=tutorial04-rgmb_abtr-rmp
         caption=ABTR RGMB Reactor Mesh Parameters example.
         block=Mesh/rmp

## Pin structures using PinMeshGenerator

[PinMeshGenerator.md] defines the constituent pin structures used for stitching into assemblies. The pin pitch, number of azimuthal sectors, and geometry / region ID information about each ring, background, and duct region are specified here.

[PinMeshGenerator.md] is called multiple times to define the various pin structures (3 fuel pin types and 1 control pin type).

!listing reactor_examples/rgmb_abtr/rgmb_abtr_het_mesh.i
         id=tutorial04-rgmb_abtr-pin
         caption=ABTR RGMB pin example.
         block=Mesh/fuel_pin_1

!alert! note title=Tips

- Use a unique [PinMeshGenerator.md] block for each pin with a unique geometrical configuration and/or region ID composition
- [!param](/Mesh/PinMeshGenerator/region_ids) is a 2-dimensional array containing region IDs (essentially materials). The first row of the array represents the 2D radial regions (from center of the pin to outermost region) for the bottom layer of the pin. Each subsequent row assigns IDs on another axial level, from bottom to top. In this case, each pin has 3 radial regions (fuel, clad, background), this array is a column pertaining to each axial level of the pin assembly.
- While the mesh is still 2D during this step, the axially dependent region IDs are stored for later use during the extrusion step.

!alert-end!

## Assembly structures using AssemblyMeshGenerator

[AssemblyMeshGenerator.md] takes the pin types previously defined and places them into a regular hexagonal grid. Additionally, coolant and duct regions need to be added around the pins in order to create the assembly geometry.

[AssemblyMeshGenerator.md] is called multiple times to define the various heterogeneous assemblies (3 fuel assemblies and 1 control assembly).

!listing reactor_examples/rgmb_abtr/rgmb_abtr_het_mesh.i
         id=tutorial04-rgmb_abtr-het-assembly
         caption=ABTR RGMB heterogeneous assembly example.
         block=Mesh/fuel_assembly_1

!alert! note title=Tips

- Use a unique [AssemblyMeshGenerator.md] block for each assembly with a unique geometrical configuration, region ID composition, and/or inventory of pin structures
- [!param](/Mesh/AssemblyMeshGenerator/background_region_id) is a 1-dimensional array containing region IDs for each axial layer of the background region. [!param](/Mesh/AssemblyMeshGenerator/duct_region_ids) is a 2-dimensional array containing regions IDs for the duct region. The first row of the array represents the 2D duct regions (from innermost to outermost duct region) for the bottom layer of the assembly. Each subsequent row assigns IDs on another axial level, from bottom to top.
- While the mesh is still 2D during this step, the axially dependent region IDs are stored for later use during the extrusion step.

!alert-end!

## Homogeneous assembly structures using PinMeshGenerator

In order to define homogeneous assembly structures to stitch into the core, we use [PinMeshGenerator.md] once again ([AssemblyMeshGenerator.md] is only used for structures that consist of lattices of pins).

- +To define single assemblies directly with PinMeshGenerators for stitching with [CoreMeshGenerator.md]+, [PinMeshGenerator.md] is used with [!param](/Mesh/PinMeshGenerator/use_as_assembly) set to `true`.
- In addition, [!param](/Mesh/PinMeshGenerator/homogenized) = `true` is used to indicate that this region is homogenized.

!listing reactor_examples/rgmb_abtr/rgmb_abtr_het_mesh.i
         id=tutorial04-rgmb_abtr-hom-assembly
         caption=ABTR RGMB homogeneous assembly example.
         block=Mesh/reflector_assembly

!alert! note title=Tips

- For homogenized assemblies, each assembly has only 1 radial region, so each column in [!param](/Mesh/PinMeshGenerator/region_ids) pertains to the region ID of each axial level of the homogenized assembly.
- While the mesh is still 2D during this step, the axially dependent region IDs are stored for later use during the extrusion step.

!alert-end!

## Heterogeneous core using CoreMeshGenerator

Now that all heterogeneous and homogeneous assemblies have been defined, they are placed into a lattice using [CoreMeshGenerator.md]. While [CoreMeshGenerator.md] still requires a perfect hexagonal pattern like [PatternedHexMeshGenerator.md], it automatically handles dummy assembly creation and deletion. The user need only provide a fake mesh input reference `dummy` (this object has not been actually created) and tell [CoreMeshGenerator.md] through the [!param](/Mesh/CoreMeshGenerator/dummy_assembly_name) parameter that the mesh input called `dummy` is a dummy assembly (empty space). The dummy assemblies will be created and deleted behind the scenes with no effort from the user.

- Since we want to extrude the 2D core, we use the [!param](/Mesh/CoreMeshGenerator/extrude)=`true` parameter within [CoreMeshGenerator.md]. The step simultaneously extrudes the geometry and applies the regions IDs to all axial layers as defined in the [PinMeshGenerator.md] and [AssemblyMeshGenerator.md] objects.
- Behind the scenes, extra element IDs `assembly_id` and `plane_id` are automatically added to the relevant elements.
- Since [!param](/Mesh/ReactorMeshParams/flexible_assembly_stitching) = `true` in [ReactorMeshParams.md], the different assembly types are stitched together without hanging nodes.

!listing reactor_examples/rgmb_abtr/rgmb_abtr_het_mesh.i
         id=tutorial04-rgmb_abtr-het_core
         caption=ABTR RGMB core example.
         block=Mesh/het_core

!alert tip
[!param](/Mesh/CoreMeshGenerator/extrude) = `true` indicates extrusion should happen at this step (this should only be set to true once in the entire input file)

!media tutorial04_meshing/rgmb_abtr_het_core_eeid.png
       id=tutorial04-rgmb_abtr_het_core
       caption=RGMB-generated 3D heterogeneous core mesh, colored by various IDs defined on the mesh.
       style=width:75%;display:block;margin-left:auto;margin-right:auto;

## Equivalent homogeneous core using Griffin's EquivalentCoreMeshGenerator

!alert! note title=Griffin executable required

This section assumes the user has access to the Griffin executable, as it invokes `EquivalentCoreMeshGenerator` that is defined within Griffin.

!alert-end!

So far, a 3-D heterogeneous pin-resolved hexagonal ABTR core geometry has been created exlusively using the Reactor Geometry Mesh Builder defined in the MOOSE Reactor module. This section requires the use of `EquivalentCoreMeshGenerator` defined in the Griffin code. This mesh generator defined in Griffin converts an input heterogeneous RGMB mesh into equivalent "duct heterogeneous", "ring heterogeneous", or "fully homogeneous" representations. Each unique subassembly region (radial + axial location) will have its own region ID in the equivalent core mesh.

- In this case, `EquivalentCoreMeshGenerator` will determine uniqueness based on both the geometry AND region ID mapping of each subassembly region in the heterogeneous core.

!listing reactor_examples/rgmb_abtr/rgmb_abtr_hom_mesh.i
         id=tutorial04-griffin_abtr-hom_core
         caption=Conversion of ABTR RGMB heterogeneous core to homogeneous core in Griffin
         block=Mesh/hom_core

!media tutorial04_meshing/rgmb_abtr_het_hom_conversion.png
       id=tutorial04-rgmb_abtr_hom_core
       caption=Side-by-side comparison of RGMB-generated heterogeneous core mesh (left) and Griffin-generated equivalent homogeneous core mesh (right), colored by region ID
       style=width:75%;display:block;margin-left:auto;margin-right:auto;

## Use of EquivalentCoreMeshGenerator Mesh with Griffin

`EquivalentCoreMeshGenerator` will define the same reporting IDs used by RGMB. In addition, `EquivalentCoreMeshGenerator` will automatically copy the "region_id" reporting ID to the "material_id" reporting ID, since Griffin recognizes material ID assignments through the `material_id` tag.

Material definition in the Griffin input file is then greatly simplified since `material_id` is defined directly on mesh. No additional mapping is needed.

!listing reactor_examples/rgmb_abtr/rgmb_abtr_griffin_snippet.i
         id=tutorial04-abtr_griffin_materials
         caption=Griffin materials setup.
         block=Materials

`EquivalentCoreMeshGenerator` and RGMB label outer boundary sidesets for core structures with pre-defined names -- "top" for top boundary, "bottom" for bottom boundary, and "outer_core" for radial boundary. Boundary conditions are assigned to these sidesets in Griffin.

!listing reactor_examples/rgmb_abtr/rgmb_abtr_griffin_snippet.i
         id=tutorial04-abtr_griffin_bcs
         caption=Griffin Boundary conditions setup.
         block=TransportSystems

!bibtex bibliography !!include to make sure next/previous are last on page

!content pagination previous=tutorial04_meshing/step10_rgmb.md
                    next=tutorial04_meshing/step12_rgmb_empire.md
