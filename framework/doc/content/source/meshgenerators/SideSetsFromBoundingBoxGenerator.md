# SideSetsFromBoundingBoxGenerator

!syntax description /Mesh/SideSetsFromBoundingBoxGenerator

## Overview

This `MeshGenerator` can be used in two ways:

-  To define a new sideset on multiple boundaries (default) - provide multiple boundary names or IDs, and all nodes existing on each provided boundary within the bounding box (or outside if `location = OUTSIDE`) will be assigned the new boundary name/ID.

- To define a new sideset *only* on overlapping boundaries (`boundary_id_overlap = True`) - provide multiple boundary names or IDs, and all nodes within the bounding box (or outside if `location = OUTSIDE`) that are currently assigned to ALL of the listed boundary names/IDs will be assigned the new boundary name/ID.

!syntax parameters /Mesh/SideSetsFromBoundingBoxGenerator

!syntax inputs /Mesh/SideSetsFromBoundingBoxGenerator

!syntax children /Mesh/SideSetsFromBoundingBoxGenerator
