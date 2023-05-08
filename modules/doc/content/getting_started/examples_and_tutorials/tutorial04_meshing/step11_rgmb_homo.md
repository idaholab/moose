# Reactor Geometry Mesh Builder Example: Homogeneous Sodium-Cooled Fast Reactor Core (ABTR)

This example illustrates the use of RGMB mesh generators to define a 3D hexagonal geometry core with homogeneous assemblies (ABTR ([!cite](shemon2015abtr)), constructed earlier in this tutorial using base mesh generators).

!media tutorial04_meshing/rgmb_abtr_stepbystep.png
       id=tutorial04-rgmb_abtr_stepbystep
       caption=Visualization of meshing steps to build the 3D ABTR core with RGMB mesh generators.
       style=width:75%;display:block;margin-left:auto;margin-right:auto;

## ReactorMeshParams

[ReactorMeshParams.md] contains global mesh/geometry parameters including whether the final mesh is 2D or 3D, Cartesian or hexagonal, assembly pitch, and the axial discretization for the final extruded geometry. This information will be accessible to the other RGMB mesh generators and consistently used.

!listing reactor_examples/rgmb_abtr/rgmb_abtr.i
         id=tutorial04-rgmb_abtr-rmp
         caption=ABTR RGMB Reactor Mesh Parameters example.
         block=Mesh/rmp

## PinMeshGenerator

This example does not have any pin-level geometry as the assemblies are homogenized. However, we still start with [PinMeshGenerator.md]. +To define single assemblies directly with PinMeshGenerators for stitching with [CoreMeshGenerator.md]+, [PinMeshGenerator.md] is used with [!param](/Mesh/PinMeshGenerator/use_as_assembly) set to `true`. This tells [CoreMeshGenerator.md] to treat this object as an assembly for stitching into the core lattice. In addition, [!param](/Mesh/PinMeshGenerator/homogenized) = `true` is used to indicate that this region is homogenized and [SimpleHexagonGenerator.md] should be called to discretize the assembly instead of [PolygonConcentricCircleMeshGenerator.md].

[PinMeshGenerator.md] is called multiple times to define the various homogeneous assemblies. Dummy assemblies are not required when building a core using RGMB, so they are not defined in this input.

!listing reactor_examples/rgmb_abtr/rgmb_abtr.i
         id=tutorial04-rgmb_abtr-assembly
         caption=ABTR RGMB assembly example.
         block=Mesh/control

Tips

- Use a unique [PinMeshGenerator.md] block for each assembly with a unique geometrical configuration and/or region ID composition
- region_ids is a 2-dimensional array containing region IDs (essentially materials). The first row of the array represents the 2D radial regions (from center of the pin to outermost region) for the bottom layer of the pin. Each subsequent row assigns IDs on another axial level, from bottom to top. In this case, each assembly has only 1 radial region, this array is a column pertaining to each axial level of the homogenized assembly.
- While the mesh is still 2D during this step, the axially dependent region IDs are stored for later use during the extrusion step.

## CoreMeshGenerator

Now that the assemblies have been defined, they are placed into a lattice using [CoreMeshGenerator.md]. While [CoreMeshGenerator.md] still requires a perfect hexagonal pattern like [PatternedHexMeshGenerator.md], it automatically handles dummy assembly creation and deletion. The user need only provide a fake mesh input reference `dummy` (this object has not been actually created) and tell [CoreMeshGenerator.md] through the [!param](/Mesh/CoreMeshGenerator/dummy_assembly_name) parameter that the mesh input called `dummy` is a dummy assembly (empty space). The dummy assemblies will be created and deleted behind the scenes with no effort from the user.

Since we want to extrude the 2D core, we use the [!param](/Mesh/CoreMeshGenerator/extrude)=`true` parameter within [CoreMeshGenerator.md]. The step simultaneously extrudes the geometry and applies the regions IDs to all axial layers as defined in the [PinMeshGenerator.md] objects.

Behind the scenes, extra element IDs `assembly_id` and `plane_id` are automatically applied.

!listing reactor_examples/rgmb_abtr/rgmb_abtr.i
         id=tutorial04-rgmb_abtr-core
         caption=ABTR RGMB core example.
         block=Mesh/core

!alert tip
[!param](/Mesh/CoreMeshGenerator/extrude) = `true` indicates extrusion should happen at this step (this should only be set to true once in the entire input file)

!media tutorial04_meshing/rgmb_abtr_core.png
       id=tutorial04-rgmb_abtr_core
       caption=RGMB-generated 3D core mesh, colored by `subdomain_id` (left), `assembly_id` (middle) and `plane_id` (right).
       style=width:75%;display:block;margin-left:auto;margin-right:auto;

!media tutorial04_meshing/rgmb_abtr_core_regionid.png
       id=tutorial04-rgmb_abtr_core_regionid
       caption=RGMB-generated 3D core mesh, colored by `region_id` at multiple axial levels.
       style=width:75%;display:block;margin-left:auto;margin-right:auto;

## Use of RGMB Mesh with Griffin

Griffin recognizes material ID assignments through the `material_id` tag. Therefore, the `region_id` tags need to be renamed to `material_id`. This is done using [ExtraElementIDCopyGenerator.md].

!listing reactor_examples/rgmb_abtr/rgmb_abtr.i
         id=tutorial04-rgmb_abtr-mesh
         caption=ABTR material ID setup example.
         block=Mesh/abtr_mesh

Material definition in the Griffin input file is then greatly simplified since `material_id` is defined directly on mesh. No additional mapping is needed.

!listing reactor_examples/rgmb_abtr/rgmb_abtr_griffin_snippet.i
         id=tutorial04-abtr_griffin_materials
         caption=Griffin materials setup.
         block=Materials

RGMB labels outer boundary sidesets with pre-defined names -- "top" for top boundary, "bottom" for bottom boundary, and "outer_core" for radial boundary. Boundary conditions are assigned to these sidesets in Griffin.

!listing reactor_examples/rgmb_abtr/rgmb_abtr_griffin_snippet.i
         id=tutorial04-abtr_griffin_bcs
         caption=Griffin Boundary conditions setup.
         block=TransportSystems

!bibtex bibliography !!include to make sure next/previous are last on page

!content pagination previous=tutorial04_meshing/step10_rgmb.md
                    next=tutorial04_meshing/step12_rgmb_hetero.md
