# BlockToMeshConverterGenerator

!syntax description /Mesh/BlockToMeshConverterGenerator

## Overview

The Block to Mesh Converter is for moving one (or more) blocks from a mesh to a new mesh. This does not erase/delete the blocks from the original mesh. The new mesh has only one block (named 0). 

It's important to note that no sidesets are preserved or made in the new mesh (neither are any unique id's or ids in general, regardless of what options are set). There are other mesh generators for generating the sidesets, such as [SideSetsAroundSubdomainGenerator](SideSetsAroundSubdomainGenerator.md) or [AllSideSetsByNormalsGenerator](AllSideSetsByNormalsGenerator.md)

## Visual Example

### Input 2D Mesh

!media large_media/framework/meshgenerators/block_to_mesh_before.png caption=A 3d object with multiple, multi-colored blocks

### Output of BlockToMeshConverterGenerator

!media large_media/framework/meshgenerators/block_to_mesh_after.png caption=A subset of blocks have been made into a new mesh

!syntax parameters /Mesh/BlockToMeshConverterGenerator

!syntax inputs /Mesh/BlockToMeshConverterGenerator

!syntax children /Mesh/BlockToMeshConverterGenerator
