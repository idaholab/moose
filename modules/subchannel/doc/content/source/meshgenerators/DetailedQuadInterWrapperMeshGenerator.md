# DetailedQuadInterWrapperMeshGenerator

!syntax description /Mesh/DetailedQuadInterWrapperMeshGenerator

## Overview

<!-- -->

This is a kernel used for visualization purposes only. It is part of an input file that creates
a 3D realistic mesh of the inter-wrapper in a square lattice arrangement. This input file has a Problem block
with the [NoSolveProblem.md] because it is not used to perform any calculations. It is used to create a detailed 3D mesh that the `SCM` solution
gets projected on.

## Example Input File Syntax

!listing /examples/mesh_generator/inter_wrapper_3d.i block=Mesh language=cpp

!syntax parameters /Mesh/DetailedQuadInterWrapperMeshGenerator

!syntax inputs /Mesh/DetailedQuadInterWrapperMeshGenerator

!syntax children /Mesh/DetailedQuadInterWrapperMeshGenerator
