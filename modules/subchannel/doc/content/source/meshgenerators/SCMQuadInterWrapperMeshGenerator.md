# SCMQuadInterWrapperMeshGenerator

!syntax description /Mesh/SCMQuadInterWrapperMeshGenerator

## Overview

!! Intentional comment to provide extra spacing

This mesh generator creates the mesh were the `SCM` solution variables associated with the inter-wrapper live. The inter-wrapper is the flow area that wraps arround the subchannel sub-assemblies.
The user needs to provide the number of subchannel sub-assemblies [!param](/Mesh/SCMQuadInterWrapperMeshGenerator/nx) in the -x direction and [!param](/Mesh/SCMQuadInterWrapperMeshGenerator/ny) in the -y direction. The number of cells in the -z direction is given by [!param](/Mesh/SCMQuadInterWrapperMeshGenerator/n_cells). The distance of the sub-assemblies from eachother is
given by the [!param](/Mesh/SCMQuadInterWrapperMeshGenerator/assembly_pitch) parameter and the total length of the inter-wrapper in the -z direction is defined by the parameters:
[!param](/Mesh/SCMQuadInterWrapperMeshGenerator/heated_length),[!param](/Mesh/SCMQuadInterWrapperMeshGenerator/unheated_length_entry),[!param](/Mesh/SCMQuadInterWrapperMeshGenerator/unheated_length_exit).
The size of each subchannel sub-assembly including the duct is defined by: [!param](/Mesh/SCMQuadInterWrapperMeshGenerator/assembly_side_x) and [!param](/Mesh/SCMQuadInterWrapperMeshGenerator/assembly_side_y). Last, [!param](/Mesh/SCMQuadInterWrapperMeshGenerator/side_bypass) is the extra size of the bypass for the side/perimetric sub-assemblies. The center of the mesh is the origin.

## Example Input File Syntax

!listing test/tests/problems/interwrapper/quad_interwrapper.i block=QuadInterWrapperMesh language=moose

!syntax parameters /Mesh/SCMQuadInterWrapperMeshGenerator

!syntax inputs /Mesh/SCMQuadInterWrapperMeshGenerator

!syntax children /Mesh/SCMQuadInterWrapperMeshGenerator
