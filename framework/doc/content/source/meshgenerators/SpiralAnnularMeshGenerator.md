# SpiralAnularMeshGenerator

## Overview

This MeshGenerator object allows the user to create an annular mesh. The nodes are located on several concentric rings, and the elements are of type Tri3 (they can be Tri6 if the user changes the value of the `use_tri6` parameter to true).

The user can choose the inner and outer radii, the number of rings, and finally the number of nodes on each ring. Given all these parameters, the radial bias will be computed automatically.

## Further SpiralAnnularMesh Documentation

!syntax parameters /MeshGenerators/SpiralAnnularMeshGenerator

!syntax inputs /MeshGenerators/SpiralAnnularMeshGenerator

!syntax children /MeshGenerators/SpiralAnnularMeshGenerator