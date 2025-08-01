# SCMTriPinMeshGenerator

!syntax description /Mesh/SCMTriPinMeshGenerator

## Overview

<!-- -->

This mesh generator creates the mesh were the `SCM` solution variables associated with the pins live.
The user needs to provide the [!param](/Mesh/SCMTriPinMeshGenerator/nrings) parameter which defines the number of fuel pin rings. For triangular sub-assemblies, one nring means that there is one central pin in the (0 0 0)
position and six neighboring ones on the the vertices of a normal hexagon. The number of cells in the -z direction is given by [!param](/Mesh/SCMTriPinMeshGenerator/n_cells). The distance of the pins from eachother is given by the [!param](/Mesh/SCMTriPinMeshGenerator/pitch) parameter and the total length of the pins in the -z direction is defined by the parameters: [!param](/Mesh/SCMTriPinMeshGenerator/heated_length),[!param](/Mesh/SCMTriPinMeshGenerator/unheated_length_entry),[!param](/Mesh/SCMTriPinMeshGenerator/unheated_length_entry).
Last, [!param](/Mesh/SCMTriPinMeshGenerator/input) is a parameter that takes the name of an object of type [SCMTriSubChannelMeshGenerator](SCMQuadSubChannelMeshGenerator.md) so the user must be careful to define consistent parameters across both `MeshGenerators`. The center of the mesh is the origin. The indexing/location of the subchannels/fuel_pins can be seen [here](general/user_notes.md)

## Example Input File Syntax

!listing /test/tests/problems/SFR/sodium-19pin/test19_monolithic.i block=TriSubChannelMesh language=moose

!syntax parameters /Mesh/SCMTriPinMeshGenerator

!syntax inputs /Mesh/SCMTriPinMeshGenerator

!syntax children /Mesh/SCMTriPinMeshGenerator
