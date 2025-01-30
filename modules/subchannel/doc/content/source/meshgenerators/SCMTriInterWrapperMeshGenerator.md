# SCMTriInterWrapperMeshGenerator

!syntax description /Mesh/SCMTriInterWrapperMeshGenerator

## Overview

<!-- -->

This kernel creates the mesh were the `SCM` solution variables associated with the inter-wrapper live. The inter-wrapper is the flow area that wraps arround the subchannel sub-assemblies.
The user needs to provide the [!param](/Mesh/SCMTriInterWrapperMeshGenerator/nrings) parameter which defines the number of sub-assemblies the inter-wrapper wraps around. For triangular sub-assemblies one nring would define
one center sub-assembly and six neighboring ones. The number of cells in the -z direction is given by [!param](/Mesh/SCMTriInterWrapperMeshGenerator/n_cells). The distance of the sub-assemblies from eachother is
given by the [!param](/Mesh/SCMTriInterWrapperMeshGenerator/assembly_pitch) parameter and the total length of the inter-wrapper in the -z direction is defined by the parameters:
[!param](/Mesh/SCMTriInterWrapperMeshGenerator/heated_length),[!param](/Mesh/SCMTriInterWrapperMeshGenerator/unheated_length_entry),[!param](/Mesh/SCMTriInterWrapperMeshGenerator/unheated_length_entry).
The size of each subchannel sub-assembly including the duct is defined by [!param](/Mesh/SCMTriInterWrapperMeshGenerator/flat_to_flat).
Last, [!param](/Mesh/SCMTriInterWrapperMeshGenerator/side_bypass) is the extra size of the bypass for the side sub-assemblies.

## Example Input File Syntax

!listing /test/tests/problems/interwrapper/tri_interwrapper_monolithic.i block=TriInterWrapperMesh language=cpp

!syntax parameters /Mesh/SCMTriInterWrapperMeshGenerator

!syntax inputs /Mesh/SCMTriInterWrapperMeshGenerator

!syntax children /Mesh/SCMTriInterWrapperMeshGenerator
