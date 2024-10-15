# DetailedQuadPinMeshGenerator

!syntax description /Mesh/DetailedQuadPinMeshGenerator

## Overview

<!-- -->

This is a kernel used for visualization purposes only. It is part of an input file that creates
a 3D realistic mesh of the pins in a square lattice arrangement. This input file has a Problem block
with `type = NoSolveProblem` because it is not used to perform any calculations. It is used to create a detailed 3D mesh that the `SCM` solution
gets projected on.

## Example Input File Syntax

!listing /examples/coupling/BISON_SC/detailedMesh.i start=GlobalParams end=AuxVariables language=cpp

!syntax parameters /Mesh/DetailedQuadPinMeshGenerator

!syntax inputs /Mesh/DetailedQuadPinMeshGenerator

!syntax children /Mesh/DetailedQuadPinMeshGenerator
