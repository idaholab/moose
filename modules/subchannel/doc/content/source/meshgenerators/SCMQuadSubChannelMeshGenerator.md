# SCMQuadSubChannelMeshGenerator

!syntax description /Mesh/SCMQuadSubChannelMeshGenerator

## Overview

<!-- -->

This mesh generator creates the mesh were the `SCM` solution variables associated with the subchannels live.
The user needs to provide the number of subchannels [!param](/Mesh/SCMQuadSubChannelMeshGenerator/nx) in the -x direction and [!param](/Mesh/SCMQuadSubChannelMeshGenerator/ny) in the -y direction.
The number of cells in the -z direction is given by [!param](/Mesh/SCMQuadSubChannelMeshGenerator/n_cells). The distance of the pins from eachother is
given by the [!param](/Mesh/SCMQuadSubChannelMeshGenerator/pitch) parameter and the total length of the sub-assembly is defined by the parameters:
[!param](/Mesh/SCMQuadSubChannelMeshGenerator/heated_length),[!param](/Mesh/SCMQuadSubChannelMeshGenerator/unheated_length_entry),[!param](/Mesh/SCMQuadSubChannelMeshGenerator/unheated_length_entry).
The fuel pin diameter is given by [!param](/Mesh/SCMQuadSubChannelMeshGenerator/pin_diameter). The user also has the ability to define the effect of spacers or mixing vanes on the subassembly
by defining their axial location [!param](/Mesh/SCMQuadSubChannelMeshGenerator/spacer_z) and a local presure from loss [!param](/Mesh/SCMQuadSubChannelMeshGenerator/spacer_k). Last the [!param](/Mesh/SCMQuadSubChannelMeshGenerator/spacer_k)
is an added distance between a perimetric pin and the duct, such that: Edge Pitch W = (pitch/2 - pin_diameter/2 + gap).

## Example Input File Syntax

!listing /test/tests/problems/psbt_transient/psbt.i block=QuadSubChannelMesh language=cpp

!syntax parameters /Mesh/SCMQuadSubChannelMeshGenerator

!syntax inputs /Mesh/SCMQuadSubChannelMeshGenerator

!syntax children /Mesh/SCMQuadSubChannelMeshGenerator
