# TriSubChannelMeshGenerator

!syntax description /Mesh/TriSubChannelMeshGenerator

## Overview

<!-- -->

This kernel creates the mesh were the `SCM` solution variables associated with the subchannels live.
The user needs to provide the [!param](/Mesh/TriSubChannelMeshGenerator/nrings) parameter which defines the number of fuel pin rings. For triangular sub-assemblies, one nring means that there is one central pin in the (0 0 0)
position and six neighboring ones on the the vertices of a normal hexagon. The number of cells in the -z direction is given by [!param](/Mesh/TriSubChannelMeshGenerator/n_cells). The distance of the pins from eachother is
given by the [!param](/Mesh/TriSubChannelMeshGenerator/pitch) parameter and the total length of the sub-assembly in the -z direction is defined by the parameters:
[!param](/Mesh/TriSubChannelMeshGenerator/heated_length),[!param](/Mesh/TriSubChannelMeshGenerator/unheated_length_entry),[!param](/Mesh/TriSubChannelMeshGenerator/unheated_length_entry).
The fuel pin diameter is given by [!param](/Mesh/TriSubChannelMeshGenerator/pin_diameter). The user also has the ability to define the effect of spacers or mixing vanes on the sub-assembly
by defining their axial location [!param](/Mesh/TriSubChannelMeshGenerator/spacer_z) and a local presure from loss [!param](/Mesh/TriSubChannelMeshGenerator/spacer_k). [!param](/Mesh/TriSubChannelMeshGenerator/flat_to_flat) is the size of the
hexagonal duct that encloses the sub-assembly. If the pins are wire wrapped then the parameters: [!param](/Mesh/TriSubChannelMeshGenerator/dwire) [!param](/Mesh/TriSubChannelMeshGenerator/hwire) have non zero values that describe the geometry of the wire-wrap.

## Example Input File Syntax

!listing /examples/Toshiba_37_pin/toshiba_37_pin.i block=TriSubChannelMesh language=cpp

!syntax parameters /Mesh/TriSubChannelMeshGenerator

!syntax inputs /Mesh/TriSubChannelMeshGenerator

!syntax children /Mesh/TriSubChannelMeshGenerator
