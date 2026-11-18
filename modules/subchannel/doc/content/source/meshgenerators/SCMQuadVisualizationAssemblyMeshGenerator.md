# SCMQuadVisualizationAssemblyMeshGenerator

!syntax description /Mesh/SCMQuadVisualizationAssemblyMeshGenerator

## Overview

!! Intentional comment to provide extra spacing

This mesh generator creates one 3D visualization mesh containing both the subchannel
volume and the fuel pin volume for a square lattice arrangement. The generated subdomains are named
`subchannel` and `fuel_pins`.

This object replaces the old chained `SCMDetailedQuadSubChannelMeshGenerator` and
`SCMDetailedQuadPinMeshGenerator` input pattern. See the
[SubChannel mesh generator migration page](modules/subchannel/general/mesh_generator_migration.md)
for old-to-new input examples.

!syntax parameters /Mesh/SCMQuadVisualizationAssemblyMeshGenerator

!syntax inputs /Mesh/SCMQuadVisualizationAssemblyMeshGenerator

!syntax children /Mesh/SCMQuadVisualizationAssemblyMeshGenerator
