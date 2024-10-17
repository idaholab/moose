# TriPinMeshGenerator

!syntax description /Mesh/TriPinMeshGenerator

## Overview

<!-- -->

This kernel creates the mesh were the `SCM` solution variables associated with the pins live.
The user needs to provide the [!param](/Mesh/TriPinMeshGenerator/nrings) parameter which defines the number of fuel pin rings. For triangular sub-assemblies, one nring means that there is one central pin in the (0 0 0)
position and six neighboring ones on the the vertices of a normal hexagon. The number of cells in the -z direction is given by [!param](/Mesh/TriPinMeshGenerator/n_cells). The distance of the pins from eachother is
given by the [!param](/Mesh/TriPinMeshGenerator/pitch) parameter and the total length of the pins in the -z direction is defined by the parameters:
[!param](/Mesh/TriPinMeshGenerator/heated_length),[!param](/Mesh/TriPinMeshGenerator/unheated_length_entry),[!param](/Mesh/TriPinMeshGenerator/unheated_length_entry).
Last, [!param](/Mesh/TriPinMeshGenerator/input) is a parameter that takes the name of an object of type [TriSubChannelMeshGenerator](QuadSubChannelMeshGenerator.md) so the user must be careful to define consistent
parameters across both `MeshGenerators`.

## Example Input File Syntax

!listing /test/tests/problems/SFR/sodium-19pin/test19_monolithic.i block=TriSubChannelMesh language=cpp

!syntax parameters /Mesh/TriPinMeshGenerator

!syntax inputs /Mesh/TriPinMeshGenerator

!syntax children /Mesh/TriPinMeshGenerator
