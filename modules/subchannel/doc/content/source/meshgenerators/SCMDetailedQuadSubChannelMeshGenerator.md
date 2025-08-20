# SCMDetailedQuadSubChannelMeshGenerator

!syntax description /Mesh/SCMDetailedQuadSubChannelMeshGenerator

## Overview

!! Intentional comment to provide extra spacing

This is a mesh generator used for visualization purposes only. It is part of an input file that creates a 3D realistic mesh of the subchannels in a square lattice arrangement. This input file has a Problem block with the [NoSolveProblem.md] because it is not used to perform any calculations. It is used to create a detailed 3D mesh that the `SCM` solution gets projected on. The center of the mesh is the origin. Last the [!param](/Mesh/SCMDetailedQuadSubChannelMeshGenerator/side_gap) is an added distance between a perimetric pin and the duct, such that: distance(edge/corner pin center, duct wall) = pitch / 2 + side_gap.

Note that:

- the vector (in the XY plane) from a corner pin center to the corner subchannel centroid is $Pitch/2 \vec{i} + Pitch/2 \vec{j}$.
- the vector (in the XY plane) from a corner pin center to the corner of the duct is $(Pitch/2 + side\_gap) \vec{i} + (Pitch/2 + side\_gap) \vec{j}$.

[side_gap] presents a sketch of the SCM geometry nomenclature near the duct corner.

!media subchannel/misc/side_gap.png
    style=width:90%;margin-bottom:2%;margin:auto;
    id=side_gap
    caption=Geometric features of the quadrilateral SCM mesh

!syntax parameters /Mesh/SCMDetailedQuadSubChannelMeshGenerator

!syntax inputs /Mesh/SCMDetailedQuadSubChannelMeshGenerator

!syntax children /Mesh/SCMDetailedQuadSubChannelMeshGenerator
