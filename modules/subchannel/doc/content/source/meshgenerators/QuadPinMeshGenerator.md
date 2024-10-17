# QuadPinMeshGenerator

!syntax description /Mesh/QuadPinMeshGenerator

## Overview

<!-- -->

This kernel creates the mesh were the `SCM` solution variables associated with the fuel pins live.
The user needs to provide the number of subchannels [!param](/Mesh/QuadPinMeshGenerator/nx) in the -x direction and [!param](/Mesh/QuadPinMeshGenerator/ny) in the -y direction.
The number of cells in the -z direction is given by [!param](/Mesh/QuadPinMeshGenerator/n_cells). The distance of the pins from eachother is
given by the [!param](/Mesh/QuadPinMeshGenerator/pitch) parameter and the total length of the pins in the -z direction is defined by the parameters:
[!param](/Mesh/QuadPinMeshGenerator/heated_length),[!param](/Mesh/QuadPinMeshGenerator/unheated_length_entry),[!param](/Mesh/QuadPinMeshGenerator/unheated_length_exit).
Last, [!param](/Mesh/QuadPinMeshGenerator/input) is a parameter that takes the name of an object of type [QuadSubChannelMeshGenerator](QuadSubChannelMeshGenerator.md) so the user must be careful to define consistent
parameters across both `MeshGenerators`.

## Example Input File Syntax

!listing /examples/psbt/psbt_ss/psbt_example.i block=QuadSubChannelMesh language=cpp

!syntax parameters /Mesh/QuadPinMeshGenerator

!syntax inputs /Mesh/QuadPinMeshGenerator

!syntax children /Mesh/QuadPinMeshGenerator
