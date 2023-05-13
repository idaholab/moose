# Reactor Geometry Mesh Builder Example: Heterogeneous Lead-Cooled Fast Reactor Assembly

This example illustrates the use of RGMB mesh generators to define a 3D pin-heterogeneous hexagonal assembly with duct. The geometry is based on an early prototype of a lead-cooled fast reactor (LFR) assembly designed by Westinghouse Electric Company (WEC) ([!cite](grasso2019lfr)). This assembly consists of 127 annular fuel pins arranged in a regular lattice of 7 rings, surrounded by coolant, a thin hexagonal duct, and an interassembly coolant gap.

!media tutorial04_meshing/rgmb_lfr_stepbystep.png
       id=tutorial04-rgmb_lfr_stepbystep
       caption=Visualization of meshing steps to build the 3D LFR assembly with RGMB mesh generators.
       style=width:100%;display:block;margin-left:auto;margin-right:auto;

## ReactorMeshParams

[ReactorMeshParams.md] contains global mesh/geometry parameters including the dimension (3D), type (hexagonal) and the axial discretization for the final geometry.

!listing reactor_examples/rgmb_lfr/rgmb_lfr_assembly.i
         id=tutorial04-rgmb_lfr-rmp
         caption=LFR RGMB Reactor Mesh Parameters example.
         block=Mesh/rmp

## PinMeshGenerator

Although all the pins are geometrically identical, each ring of pins may use different material multigroup cross sections in later neutronics calculations. We select to distinguish pins by ring (7 rings) by creating a [PinMeshGenerator.md] instance for each ring. If the materials (cross sections) are identical across all pins, only one pin definition would be necessary. The [!param](/Mesh/PinMeshGenerator/pin_type) parameter should be unique for each pin type (1 through 7).

The pitch of the pin is specified with pitch, and the number of azimuthal sectors is specified with [!param](/Mesh/PinMeshGenerator/num_sectors). The number or radial blocks and radial sub-intevals is specified through the array [!param](/Mesh/PinMeshGenerator/mesh_intervals). In this case, the pin has 4 important radii corresponding to the central helium gap radius, outer fuel radius, inner cladding radius, and outer cladding radius. These input parameters differ slightly from [PolygonConcentricCircleMeshGenerator.md], and no advanced options like boundary layer meshing are exposed currently.

!media tutorial04_meshing/lfr_pinmesh.png
       id=tutorial04-lfr_pinmesh
       caption=Example LFR assembly pin cell mesh colored by `subdomain_id`.
       style=width:50%;display:block;margin-left:auto;margin-right:auto;

!listing reactor_examples/rgmb_lfr/rgmb_lfr_assembly.i
         id=tutorial04-rgmb_lfr-pin1
         caption=LFR RGMB pin example.
         block=Mesh/pin1

[!param](/Mesh/PinMeshGenerator/region_ids) is a 2-dimensional array where each row assigns to radial layer `region_id`s on a given axial layer, with the bottom layer defined first. Each pin has multiple radial regions (hole, fuel, clad, coolant) which must be defined for each axial layer.

## AssemblyMeshGenerator

[AssemblyMeshGenerator.md] takes the 7 pin types previously defined and places them into a regular hexagonal grid. Additionally, coolant and duct regions need to be added around the pins in order to create the assembly geometry.

The assembly pitch has already been defined in [ReactorMeshParams.md] and forms the outer edge of the assembly duct. The inner half pitch of the duct should be specified using the [!param](/Mesh/AssemblyMeshGenerator/duct_halfpitch) parameter. Additional duct blocks can be created by adding more radii values to this parameter, remembering that each value must be less than half of the [!param](/Mesh/ReactorMeshParams/assembly_pitch) parameter. The radial meshing subintervals for the duct are defined with the [!param](/Mesh/AssemblyMeshGenerator/duct_intervals) option.

Whatever area remains between the outer pin boundaries and the inner half pitch of the duct is filled in with "background" region (representing coolant, in this case). The radial meshing subintervals for the coolant are defined with the [!param](/Mesh/AssemblyMeshGenerator/background_intervals) option.

Extrusion is performed by using the [!param](/Mesh/AssemblyMeshGenerator/extrude)=`true` option and is performed after the 2D assembly geometry has been completed.

!listing reactor_examples/rgmb_lfr/rgmb_lfr_assembly.i
         id=tutorial04-rgmb_lfr-assembly
         caption=LFR RGMB assembly example.
         block=Mesh/assembly

RGMB allows assignment of background coolant and duct region IDs which are added when pins are patterned into an assembly lattice.

!media tutorial04_meshing/rgmb_lfr_assembly.png
       id=tutorial04-rgmb_lfr_assembly
       caption=RGMB-generated LFR assembly mesh colored by `subdomain_id` (left), `pin_id` (middle) and `plane_id` (right).
       style=width:75%;display:block;margin-left:auto;margin-right:auto;

!media tutorial04_meshing/rgmb_lfr_assembly_matid.png
       id=tutorial04-rgmb_lfr_assembly_matid
       caption=`material_id` Reporting ID assignment.
       style=width:75%;display:block;margin-left:auto;margin-right:auto;

## Use of RGMB Mesh with Griffin

`region_id` extra element integer needs to be renamed to `material_id` so that Griffin can recognize these values are material assignments.

!listing reactor_examples/rgmb_lfr/rgmb_lfr_assembly.i
         id=tutorial04-rgmb_lfr-material-id
         caption=LFR RGMB Material ID setup.
         block=Mesh/lfr_assy

Boundary conditions are assigned to the outer boundary sidesets. The default outer boundary name for an assembly with [!param](/Mesh/AssemblyMeshGenerator/assembly_type) `1` is `outer_assembly_1`.

!listing reactor_examples/rgmb_lfr/rgmb_lfr_griffin_snippet.i
         id=tutorial04-rgmb_lfr-transport
         caption=LFR RGMB boundary condition setup.
         block=TransportSystems

Material definition is greatly simplified since the `material_id` extra element ID is defined directly on mesh. Just the blocks need to be listed. The remainder of the file shown here simply describes the composition of the materials by isotope, and where to find them in the library file. There is no mapping performed in Griffin.

!listing reactor_examples/rgmb_lfr/rgmb_lfr_griffin_snippet.i
         id=tutorial04-rgmb_lfr-materials
         caption=LFR RGMB materials setup.
         block=Materials

!bibtex bibliography !!include to make sure next/previous are last on page

!content pagination previous=tutorial04_meshing/step11_rgmb_homo.md
                    next=tutorial04_meshing/step13_advanced_tools.md
