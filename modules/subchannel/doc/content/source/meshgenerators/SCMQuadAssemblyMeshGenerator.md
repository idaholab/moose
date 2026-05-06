# SCMQuadAssemblyMeshGenerator

!syntax description /Mesh/SCMQuadAssemblyMeshGenerator

## Overview

!! Intentional comment to provide extra spacing

This mesh generator creates the 1D subchannel and pin meshes for a square lattice arrangement.
The generated subdomains are named `subchannel` and `fuel_pins`.
The center of the mesh is the origin.

This object replaces the old chained `SCMQuadSubChannelMeshGenerator` and
`SCMQuadPinMeshGenerator` input pattern. See the
[SubChannel mesh generator migration page](modules/subchannel/general/mesh_generator_migration.md)
for old-to-new input examples.

## Example Input File Syntax

!listing /test/tests/problems/psbt/psbt_explicit.i block=QuadSubChannelMesh language=moose

!syntax parameters /Mesh/SCMQuadAssemblyMeshGenerator

!syntax inputs /Mesh/SCMQuadAssemblyMeshGenerator

!syntax children /Mesh/SCMQuadAssemblyMeshGenerator
