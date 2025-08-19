# SCMDetailedQuadSubChannelMeshGenerator

!syntax description /Mesh/SCMDetailedQuadSubChannelMeshGenerator

## Overview

!! Intentional comment to provide extra spacing

This is a mesh generator used for visualization purposes only. It is part of an input file that creates a 3D realistic mesh of the subchannels in a square lattice arrangement. This input file has a Problem block with the [NoSolveProblem.md] because it is not used to perform any calculations. It is used to create a detailed 3D mesh that the `SCM` solution gets projected on. Last the [!param](/Mesh/SCMDetailedQuadSubChannelMeshGenerator/side_gap) is an added distance between a perimetric pin and the duct, such that: distance(edge pin center, duct wall) = pitch / 2 + side_gap. Note that the distance (in the XY plane) from a corner/edge subchannel centroid and the nearest fuel pin center is pitch/2. Also note that the distance (in the XY plane) from a corner/edge subchannel centroid to the nearest duct wall is [!param](/Mesh/SCMDetailedQuadSubChannelMeshGenerator/side_gap). The center of the mesh is the origin.

!syntax parameters /Mesh/SCMDetailedQuadSubChannelMeshGenerator

!syntax inputs /Mesh/SCMDetailedQuadSubChannelMeshGenerator

!syntax children /Mesh/SCMDetailedQuadSubChannelMeshGenerator
