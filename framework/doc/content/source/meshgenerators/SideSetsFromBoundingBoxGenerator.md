# SideSetsFromBoundingBoxGenerator

!syntax description /MeshGenerators/SideSetsFromBoundingBoxGenerator

## Overview

This `MeshGenerator` can be used in two ways:

-  To define a new sideset on multiple boundaries (default) - provide multiple boundary IDs, and all nodes existing on each provided boundary within the bounding box (or outside if `location = OUTSIDE`) will be assigned the new boundary ID.

- To define a new sideset *only* on overlapping boundaries (`boundary_id_overlap = True`) - provide multiple boundary IDs, and all nodes within the bounding box (or outside if `location = OUTSIDE`) that are currently assigned to ALL of the listed boundary IDs will be assigned the new boundary ID.

!syntax parameters /MeshGenerators/SideSetsFromBoundingBoxGenerator

!syntax inputs /MeshGenerators/SideSetsFromBoundingBoxGenerator

!syntax children /MeshGenerators/SideSetsFromBoundingBoxGenerator
