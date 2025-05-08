# RGMB Example: Heterogeneous to Homogeneous Conversion for Fast Reactor Core (ABTR)

!---

## Conversion of Heterogeneous to Homogeneous Sodium-Cooled Fast Reactor Core Mesh (ABTR)

This example illustrates the use of RGMB mesh generators to define a pin-resolved heterogeneous 3D hexagonal geometry core (ABTR ([!cite](shemon2015abtr)), and using Griffin to convert this mesh to an equivalent homogenized core mesh with automatic geometry construction and region ID assignment. The final mesh constructed will be similar to the one presented earlier in this tutorial using base mesh generators.

!media tutorial04_meshing/rgmb_abtr_hethom_stepbystep.png
       style=width:50%;display:block;margin-left:auto;margin-right:auto;

+Hands-on package MOOSE input file (Heterogeneous ABTR mesh)+: `tutorials/tutorial04_meshing/doc/listings/reactor_examples/rgmb_abtr/rgmb_abtr_het_mesh.i`

+Hands-on package Griffin input file (Equivalent homogeneous ABTR mesh, requires Griffin executable)+: `tutorials/tutorial04_meshing/doc/listings/reactor_examples/rgmb_abtr/rgmb_abtr_hom_mesh.i`

!---

## ReactorMeshParams

[ReactorMeshParams.md] contains global mesh/geometry parameters including whether the final mesh is 2D or 3D, Cartesian or hexagonal, assembly pitch, and the axial discretization for the final extruded geometry. This information will be accessible to the other RGMB mesh generators and consistently used. Here we also invoke the option to enable flexible assembly stitching, so that dissimilar assembly structures can be stitched into the reactor core without any hanging nodes

!listing reactor_examples/rgmb_abtr/rgmb_abtr_het_mesh.i
         block=Mesh/rmp
         link=False

!---

## Pin structures using PinMeshGenerator

[PinMeshGenerator.md] defines the constituent pin structures used for stitching into assemblies. The pin pitch, number of azimuthal sectors, and geometry / region ID information about each ring, background, and duct region are specified here.

[PinMeshGenerator.md] is called multiple times to define the various pin structures (3 fuel pin types and 1 control pin type).

!listing reactor_examples/rgmb_abtr/rgmb_abtr_het_mesh.i
         block=Mesh/fuel_pin_1
         link=False

!---

## Assembly structures using AssemblyMeshGenerator

[AssemblyMeshGenerator.md] takes the pin types previously defined and places them into a regular hexagonal grid. Additionally, coolant and duct regions need to be added around the pins in order to create the assembly geometry.

[AssemblyMeshGenerator.md] is called multiple times to define the various heterogeneous assemblies (3 fuel assemblies and 1 control assembly).

!listing reactor_examples/rgmb_abtr/rgmb_abtr_het_mesh.i
         block=Mesh/fuel_assembly_1
         link=False

!---

## Homogeneous assembly structures using PinMeshGenerator

In order to define homogeneous assembly structures to stitch into the core, we use [PinMeshGenerator.md] once again ([AssemblyMeshGenerator.md] is only used for structures that consist of lattices of pins).

- +To define single assemblies directly with PinMeshGenerators for stitching with [CoreMeshGenerator.md]+, [PinMeshGenerator.md] is used with [!param](/Mesh/PinMeshGenerator/use_as_assembly) set to `true`.
- In addition, [!param](/Mesh/PinMeshGenerator/homogenized) = `true` is used to indicate that this region is homogenized.

!listing reactor_examples/rgmb_abtr/rgmb_abtr_het_mesh.i
         block=Mesh/reflector_assembly
         link=False

!---


## Heterogeneous core using CoreMeshGenerator

[CoreMeshGenerator.md] takes the input assemblies (heterogeneous and homogeneous) and stitches them into a core.

- Since [!param](/Mesh/ReactorMeshParams/flexible_assembly_stitching) = `true` in [ReactorMeshParams.md], the different assembly types are stitched together without hanging nodes.
- Dummy assemblies do not need to be defined explicitly and instead can be referenced in the core lattice input by specifying a name in [!param](/Mesh/CoreMeshGenerator/dummy_assembly_name).
- This is the last step you can run if you do not have Griffin executable access.

!row!

!col! width=50%

!listing reactor_examples/rgmb_abtr/rgmb_abtr_het_mesh.i
         block=Mesh/het_core
         link=False

!col-end!

!col! width=50%

!media tutorial04_meshing/rgmb_abtr_het_core_eeid.png
       style=width:100%;display:block;margin-left:auto;margin-right:auto;

!col-end!

!row-end!

!---

## Equivalent homogeneous core using Griffin's EquivalentCoreMeshGenerator

`EquivalentCoreMeshGenerator` is a mesh generator defined in Griffin that converts an input heterogeneous RGMB mesh into equivalent "duct heterogeneous", "ring heterogeneous", or "fully homogeneous" representations. Each unique subassembly region (radial + axial location) will have its own region ID in the equivalent core mesh. This step requires Griffin executable access.

In this case, `EquivalentCoreMeshGenerator` will determine uniqueness based on both the geometry AND region ID mapping of each subassembly region in the heterogeneous core.


!row!
!col! width=50%

!listing reactor_examples/rgmb_abtr/rgmb_abtr_hom_mesh.i
         block=Mesh/hom_core
         link=False

!col-end!

!col! width=50%

!media tutorial04_meshing/rgmb_abtr_het_hom_conversion.png
       style=width:100%;display:block;margin-left:auto;margin-right:auto;

!col-end!

!row-end!

!---

## Use of EquivalentCoreMeshGenerator with Griffin

`EquivalentCoreMeshGenerator` will define the same reporting IDs used by RGMB.

In addition, `EquivalentCoreMeshGenerator` will automatically copy the "region_id" reporting ID to the "material_id" reporting ID, since Griffin recognizes material ID assignments through the `material_id` tag.

Material definition in the Griffin input file is then greatly simplified since `material_id` is defined directly on mesh. No additional mapping is needed.

!listing reactor_examples/rgmb_abtr/rgmb_abtr_griffin_snippet.i
         block=Materials
         link=False

!---

`EquivalentCoreMeshGenerator` and RGMB label outer boundary sidesets for core structures with pre-defined names -- "top" for top boundary, "bottom" for bottom boundary, and "outer_core" for radial boundary. Boundary conditions are assigned to these sidesets in Griffin.

!listing reactor_examples/rgmb_abtr/rgmb_abtr_griffin_snippet.i
         block=TransportSystems
         link=False
