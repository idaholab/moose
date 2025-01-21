# DetailedTriSubChannelMeshGenerator

!syntax description /Mesh/DetailedTriSubChannelMeshGenerator

## Overview

<!-- -->

This is a kernel used for visualization purposes only. It is part of an input file that creates
a 3D realistic mesh of the subchannels in a triangular lattice arrangement. This input file has a Problem block
with the [NoSolveProblem.md] because it is not used to perform any calculations. It is used to create a detailed 3D mesh that the `SCM` solution
gets projected on.

## Example Input File Syntax

!listing /test/tests/problems/Lead-LBE-19pin/3D_LBE-19pin.i block=Mesh language=cpp

!syntax parameters /Mesh/DetailedTriSubChannelMeshGenerator

!syntax inputs /Mesh/DetailedTriSubChannelMeshGenerator

!syntax children /Mesh/DetailedTriSubChannelMeshGenerator
