# SCMDetailedQuadPinMeshGenerator

!syntax description /Mesh/SCMDetailedQuadPinMeshGenerator

## Overview

<!-- -->

This is a mesh generator used for visualization purposes only. It is part of an input file that creates
a 3D realistic mesh of the pins in a square lattice arrangement. This input file has a Problem block
with the [NoSolveProblem.md] because it is not used to perform any calculations. It is used to create a detailed 3D mesh that the `SCM` solution
gets projected on.
Last, [!param](/Mesh/SCMDetailedQuadPinMeshGenerator/input) is a parameter that takes the name of an object of type [SCMDetailedQuadSubChannelMeshGenerator](SCMDetailedQuadSubChannelMeshGenerator.md) so the user must be careful to define consistent
parameters across both `MeshGenerators`.

## Example Input File Syntax

!listing examples/coupling/HeatConduction_SCM/detailedMesh.i start=GlobalParams end=AuxVariables language=cpp

!syntax parameters /Mesh/SCMDetailedQuadPinMeshGenerator

!syntax inputs /Mesh/SCMDetailedQuadPinMeshGenerator

!syntax children /Mesh/SCMDetailedQuadPinMeshGenerator
