# SCMDetailedQuadInterWrapperMeshGenerator

!syntax description /Mesh/SCMDetailedQuadInterWrapperMeshGenerator

## Overview

!! Intentional comment to provide extra spacing

This is a mesh generator used for visualization purposes only. It is part of an input file that creates
a 3D realistic mesh of the inter-wrapper in a square lattice arrangement. This input file has a Problem block
with the [NoSolveProblem.md] because it is not used to perform any calculations. It is used to create a detailed 3D mesh that the `SCM` solution gets projected on.

## Example Input File Syntax

!listing /examples/mesh_generator/inter_wrapper_3d.i block=Mesh language=moose

!syntax parameters /Mesh/SCMDetailedQuadInterWrapperMeshGenerator

!syntax inputs /Mesh/SCMDetailedQuadInterWrapperMeshGenerator

!syntax children /Mesh/SCMDetailedQuadInterWrapperMeshGenerator
