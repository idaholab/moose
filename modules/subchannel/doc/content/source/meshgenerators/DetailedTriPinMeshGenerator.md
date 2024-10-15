# DetailedTriPinMeshGenerator

!syntax description /Mesh/DetailedTriPinMeshGenerator

## Overview

<!-- -->

This is a kernel used for visualization purposes only. It is part of an input file that creates
a 3D realistic mesh of the pins in a triangular lattice arrangement. This input file has a Problem block
with `type = NoSolveProblem` because it is not used to perform any calculations. It is used to create a detailed 3D mesh that the `SCM` solution
gets projected on.

## Example Input File Syntax

!listing /examples/full-sodium-assembly/3d.i block=Mesh language=cpp

!syntax parameters /Mesh/DetailedTriPinMeshGenerator

!syntax inputs /Mesh/DetailedTriPinMeshGenerator

!syntax children /Mesh/DetailedTriPinMeshGenerator
