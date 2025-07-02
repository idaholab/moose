# SCMDetailedTriSubChannelMeshGenerator

!syntax description /Mesh/SCMDetailedTriSubChannelMeshGenerator

## Overview

<!-- -->

This is a mesh generator used for visualization purposes only. It is part of an input file that creates
a 3D realistic mesh of the subchannels in a triangular lattice arrangement. This input file has a Problem block
with the [NoSolveProblem.md] because it is not used to perform any calculations. It is used to create a detailed 3D mesh that the `SCM` solution gets projected on. The center of the mesh is the origin.

## Example Input File Syntax

!listing /test/tests/problems/Lead-LBE-19pin/3D_LBE-19pin.i block=Mesh language=moose

!syntax parameters /Mesh/SCMDetailedTriSubChannelMeshGenerator

!syntax inputs /Mesh/SCMDetailedTriSubChannelMeshGenerator

!syntax children /Mesh/SCMDetailedTriSubChannelMeshGenerator
