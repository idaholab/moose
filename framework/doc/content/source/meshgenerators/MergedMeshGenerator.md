# MergedMeshGenerator

## Overview

The `MergedMeshGenerator` object allows for multiple mesh files to be "merged"
together to form a single mesh for use in a simulation. This generator will keep
all submeshes disconnected from each other. Interaction between submeshes has to
come from geometric contact (mechanical or thermal).

## Further MergedMeshGenerator Documentation

!syntax parameters /MeshGenerators/MergedMeshGenerator

!syntax inputs /MeshGenerators/MergedMeshGenerator

!syntax children /MeshGenerators/MergedMeshGenerator
