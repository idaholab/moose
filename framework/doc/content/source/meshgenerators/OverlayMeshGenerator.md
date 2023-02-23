# OverlayMeshGenerator

!syntax description /Mesh/OverlayMeshGenerator

## Overview

The `OverlayMeshGenerator` object is the built-in mesh generation capable of creating an overlaying mesh with the given mesh block. The overlay mesh uses DistributedRectilinearMeshGenerator(DistributedRectilinearMeshGenerator.md) as sub-generator. The input parameters for DistributedRectilinearMeshGenerator are all available for OverlayMeshGenerator. The required input parameters are [!param](/Mesh/OverlayMeshGenerator/dim) (the dimension of the domain) and [!param](/Mesh/OverlayMeshGenerator/input) (the base mesh we want to overlay).

## Example Syntax

!listing test/tests/meshgenerators/overlay_mesh_generator/overlay_mesh_generator.i
         block=Mesh

!syntax parameters /Mesh/OverlayMeshGenerator

!syntax inputs /Mesh/OverlayMeshGenerator

!syntax children /Mesh/OverlayMeshGenerator
