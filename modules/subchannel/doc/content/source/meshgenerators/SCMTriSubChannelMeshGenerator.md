# SCMTriSubChannelMeshGenerator

!syntax description /Mesh/SCMTriSubChannelMeshGenerator

## Overview

<!-- -->

This mesh generator creates the mesh were the `SCM` solution variables associated with the subchannels live.
The user needs to provide the [!param](/Mesh/SCMTriSubChannelMeshGenerator/nrings) parameter which defines the number of fuel pin rings. For triangular sub-assemblies, one nring means that there is one central pin in the (0 0 0) position and six neighboring ones on the the vertices of a normal hexagon. The number of cells in the -z direction is given by [!param](/Mesh/SCMTriSubChannelMeshGenerator/n_cells). The distance of the pins from eachother is given by the [!param](/Mesh/SCMTriSubChannelMeshGenerator/pitch) parameter and the total length of the sub-assembly in the -z direction is defined by the parameters:
[!param](/Mesh/SCMTriSubChannelMeshGenerator/heated_length),[!param](/Mesh/SCMTriSubChannelMeshGenerator/unheated_length_entry),[!param](/Mesh/SCMTriSubChannelMeshGenerator/unheated_length_entry).
The fuel pin diameter is given by [!param](/Mesh/SCMTriSubChannelMeshGenerator/pin_diameter). The user also has the ability to define the effect of spacers or mixing vanes on the sub-assembly
by defining their axial location [!param](/Mesh/SCMTriSubChannelMeshGenerator/spacer_z) and a local presure from loss [!param](/Mesh/SCMTriSubChannelMeshGenerator/spacer_k). [!param](/Mesh/SCMTriSubChannelMeshGenerator/flat_to_flat) is the size of the hexagonal duct that encloses the sub-assembly. If the pins are wire wrapped then the parameters: [!param](/Mesh/SCMTriSubChannelMeshGenerator/dwire) [!param](/Mesh/SCMTriSubChannelMeshGenerator/hwire) have non zero values that describe the geometry of the wire-wrap.
The center of the mesh is the origin. The indexing/location of the subchannels/fuel_pins can be seen [here](general/user_notes.md)

## Example Input File Syntax

!listing /validation/Toshiba_37_pin/toshiba_37_pin.i block=TriSubChannelMesh language=moose

!syntax parameters /Mesh/SCMTriSubChannelMeshGenerator

!syntax inputs /Mesh/SCMTriSubChannelMeshGenerator

!syntax children /Mesh/SCMTriSubChannelMeshGenerator
