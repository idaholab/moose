# SCMTriAssemblyMeshGenerator

!syntax description /Mesh/SCMTriAssemblyMeshGenerator

## Overview

!! Intentional comment to provide extra spacing

This mesh generator creates the 1D subchannel and pin meshes for a triangular lattice arrangement.
The generated subdomains are named `subchannel` and `fuel_pins`.
The center of the mesh is the origin.

This object replaces the old chained `SCMTriSubChannelMeshGenerator` and
`SCMTriPinMeshGenerator` input pattern. See the
[SubChannel mesh generator migration page](modules/subchannel/general/mesh_generator_migration.md)
for old-to-new input examples.

## Example Input File Syntax

!listing /test/tests/problems/SFR/sodium-19pin/test19_monolithic.i block=TriSubChannelMesh language=moose

!syntax parameters /Mesh/SCMTriAssemblyMeshGenerator

!syntax inputs /Mesh/SCMTriAssemblyMeshGenerator

!syntax children /Mesh/SCMTriAssemblyMeshGenerator
