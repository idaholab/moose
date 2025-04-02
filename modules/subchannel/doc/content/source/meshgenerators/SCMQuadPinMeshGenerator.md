# SCMQuadPinMeshGenerator

!syntax description /Mesh/SCMQuadPinMeshGenerator

## Overview

<!-- -->

This mesh generator creates the mesh were the `SCM` solution variables associated with the fuel pins live.
The user needs to provide the number of subchannels [!param](/Mesh/SCMQuadPinMeshGenerator/nx) in the -x direction and [!param](/Mesh/SCMQuadPinMeshGenerator/ny) in the -y direction.
The number of cells in the -z direction is given by [!param](/Mesh/SCMQuadPinMeshGenerator/n_cells). The distance of the pins from eachother is given by the [!param](/Mesh/SCMQuadPinMeshGenerator/pitch) parameter and the total length of the pins in the -z direction is defined by the parameters: [!param](/Mesh/SCMQuadPinMeshGenerator/heated_length),[!param](/Mesh/SCMQuadPinMeshGenerator/unheated_length_entry),[!param](/Mesh/SCMQuadPinMeshGenerator/unheated_length_exit). Last, [!param](/Mesh/SCMQuadPinMeshGenerator/input) is a parameter that takes the name of an object of type [SCMQuadSubChannelMeshGenerator](SCMQuadSubChannelMeshGenerator.md) so the user must be careful to define consistent parameters across both `MeshGenerators`. The center of the mesh is the origin.

## Example Input File Syntax

!listing /v&v/psbt/psbt_ss/psbt.i block=QuadSubChannelMesh language=cpp

!syntax parameters /Mesh/SCMQuadPinMeshGenerator

!syntax inputs /Mesh/SCMQuadPinMeshGenerator

!syntax children /Mesh/SCMQuadPinMeshGenerator
