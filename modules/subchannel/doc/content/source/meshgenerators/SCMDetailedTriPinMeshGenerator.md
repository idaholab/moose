# SCMDetailedTriPinMeshGenerator

!syntax description /Mesh/SCMDetailedTriPinMeshGenerator

## Overview

<!-- -->

This is a mesh generator used for visualization purposes only. It is part of an input file that creates
a 3D realistic mesh of the pins in a triangular lattice arrangement. This input file has a Problem block
with the [NoSolveProblem.md] because it is not used to perform any calculations. It is used to create a detailed 3D mesh that the `SCM` solution gets projected on. Last, [!param](/Mesh/SCMDetailedTriPinMeshGenerator/input) is a parameter that takes the name of an object of type [SCMDetailedTriSubChannelMeshGenerator](SCMDetailedTriSubChannelMeshGenerator.md) so the user must be careful to define consistent parameters across both `MeshGenerators`. The center of the mesh is the origin.

!syntax parameters /Mesh/SCMDetailedTriPinMeshGenerator

!syntax inputs /Mesh/SCMDetailedTriPinMeshGenerator

!syntax children /Mesh/SCMDetailedTriPinMeshGenerator
