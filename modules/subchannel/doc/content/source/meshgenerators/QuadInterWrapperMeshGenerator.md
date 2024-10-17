# QuadInterWrapperMeshGenerator

!syntax description /Mesh/QuadInterWrapperMeshGenerator

## Overview

<!-- -->

This kernel creates the mesh were the `SCM` solution variables associated with the inter-wrapper live. The inter-wrapper is the flow area that wraps arround the subchannel sub-assemblies.
The user needs to provide the number of subchannel sub-assemblies [!param](/Mesh/QuadInterWrapperMeshGenerator/nx) in the -x direction and [!param](/Mesh/QuadInterWrapperMeshGenerator/ny) in the -y direction.
The number of cells in the -z direction is given by [!param](/Mesh/QuadInterWrapperMeshGenerator/n_cells). The distance of the sub-assemblies from eachother is
given by the [!param](/Mesh/QuadInterWrapperMeshGenerator/assembly_pitch) parameter and the total length of the inter-wrapper in the -z direction is defined by the parameters:
[!param](/Mesh/QuadInterWrapperMeshGenerator/heated_length),[!param](/Mesh/QuadInterWrapperMeshGenerator/unheated_length_entry),[!param](/Mesh/QuadInterWrapperMeshGenerator/unheated_length_exit).
The size of each subchannel sub-assembly including the duct is defined by: [!param](/Mesh/QuadInterWrapperMeshGenerator/assembly_side_x) and [!param](/Mesh/QuadInterWrapperMeshGenerator/assembly_side_y).
Last, [!param](/Mesh/QuadInterWrapperMeshGenerator/side_bypass) is the extra size of the bypass for the side/perimetric sub-assemblies.

## Example Input File Syntax

!listing test/tests/problems/interwrapper/quad_interwrapper.i block=QuadInterWrapperMesh language=cpp

!syntax parameters /Mesh/QuadInterWrapperMeshGenerator

!syntax inputs /Mesh/QuadInterWrapperMeshGenerator

!syntax children /Mesh/QuadInterWrapperMeshGenerator
