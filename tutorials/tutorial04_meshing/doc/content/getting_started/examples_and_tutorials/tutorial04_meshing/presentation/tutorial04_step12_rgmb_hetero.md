# Reactor Geometry Mesh Builder Example: Heterogeneous Lead-Cooled Fast Reactor Assembly

!---

## Heterogeneous Lead-Cooled Fast Reactor Assembly

This example illustrates the use of RGMB mesh generators to define a 3D pin-heterogeneous hexagonal assembly with duct. The geometry is based on an early prototype of a lead-cooled fast reactor (LFR) assembly designed by Westinghouse Electric Company (WEC) ([!cite](grasso2019lfr)).

!media tutorial04_meshing/rgmb_lfr_stepbystep.png
       style=width:100%;display:block;margin-left:auto;margin-right:auto;

+Hands-on package MOOSE input file+: `combined/reactor_workshop/tests/reactor_examples/rgmb_lfr/rgmb_lfr_assembly.i`

!---

## ReactorMeshParams

[ReactorMeshParams.md] contains global mesh/geometry parameters including the dimension (3D), type (hexagonal) and the axial discretization for the final geometry.

!listing reactor_examples/rgmb_lfr/rgmb_lfr_assembly.i
         block=Mesh/rmp
         link=False

!---

## PinMeshGenerator

The pitch of the pin is specified with pitch, and the number of azimuthal sectors is specified with [!param](/Mesh/PinMeshGenerator/num_sectors). The number or radial blocks and radial sub-intevals is specified through the array [!param](/Mesh/PinMeshGenerator/mesh_intervals). In this case, the pin has 4 important radii corresponding to the central helium gap radius, outer fuel radius, inner cladding radius, and outer cladding radius.

!row!
!col small=12 medium=6 large=8

!listing reactor_examples/rgmb_lfr/rgmb_lfr_assembly.i
         block=Mesh/pin1
         link=False

!col small=12 medium=6 large=4

!media tutorial04_meshing/lfr_pinmesh.png
       style=width:100%;display:block;margin-left:auto;margin-right:auto;

!row-end!

!---

## AssemblyMeshGenerator

[AssemblyMeshGenerator.md] takes the pin types previously defined and places them into a regular hexagonal grid. Additionally, coolant and duct regions need to be added around the pins in order to create the assembly geometry.

Extrusion is performed by using the [!param](/Mesh/AssemblyMeshGenerator/extrude)=`true` option and is performed after the 2D assembly geometry has been completed.

!listing reactor_examples/rgmb_lfr/rgmb_lfr_assembly.i
         block=Mesh/assembly
         link=False

!---

EEIDs for `subdomain_id` (left), `pin_id` (middle) and `plane_id` (right) automatically applied:

!media tutorial04_meshing/rgmb_lfr_assembly.png
       style=width:60%;display:block;margin-left:auto;margin-right:auto;

Materials IDs also assigned:

!media tutorial04_meshing/rgmb_lfr_assembly_matid.png
       style=width:60%;display:block;margin-left:auto;margin-right:auto;

!---

## Use of RGMB Mesh with Griffin

`region_id` extra element integer needs to be renamed to `material_id` so that Griffin can recognize these values are material assignments

!listing reactor_examples/rgmb_lfr/rgmb_lfr_assembly.i
         block=Mesh/lfr_assy
         link=False

Boundary conditions are assigned to the outer boundary sidesets. The default outer boundary name for an assembly with [!param](/Mesh/AssemblyMeshGenerator/assembly_type) `1` is `outer_assembly_1`.

!listing reactor_examples/rgmb_lfr/rgmb_lfr_griffin_snippet.i
         block=TransportSystems
         link=False

!---

Material definition is greatly simplified since the `material_id` extra element ID is defined directly on mesh. Just the blocks need to be listed. The remainder of the file shown here simply describes the composition of the materials by isotope, and where to find them in the library file. There is no mapping performed in Griffin.

!listing reactor_examples/rgmb_lfr/rgmb_lfr_griffin_snippet.i
         block=Materials
         link=False
