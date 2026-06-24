# SCMDetailedTriAssemblyMeshGenerator

!syntax description /Mesh/SCMDetailedTriAssemblyMeshGenerator

## Overview

!! Intentional comment to provide extra spacing

This mesh generator creates one detailed 3D visualization mesh containing both the subchannel
volume and the fuel pin volume for a triangular lattice arrangement. The generated subdomains are
named `subchannel` and `fuel_pins`.

This object replaces the old chained `SCMDetailedTriSubChannelMeshGenerator` and
`SCMDetailedTriPinMeshGenerator` input pattern. See the
[SubChannel mesh generator migration page](modules/subchannel/general/mesh_generator_migration.md)
for old-to-new input examples.

!syntax parameters /Mesh/SCMDetailedTriAssemblyMeshGenerator

!syntax inputs /Mesh/SCMDetailedTriAssemblyMeshGenerator

!syntax children /Mesh/SCMDetailedTriAssemblyMeshGenerator
