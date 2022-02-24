# BoundaryPreservedMarker

## Description

This is the same as [ErrorFractionMarker](ErrorFractionMarker.md), which
marks elements for refinement or coarsening based on the fraction of the min/max error
from the supplied indicator while preserving the given boundary.

The motivation is to preserve the boundary geometry during the mesh coarsening.
Any elements that connect with the boundary will be maintained during coarsening.
These elements might be coarsened later if the boundary moves to a different location.

## Example Input File Syntax

!listing test/tests/userobjects/element_subdomain_modifier/adaptivity_moving_boundary.i
           block=Adaptivity
           link=False

!! Describe and include an example of how to use the BoundaryPreservedMarker object.

!syntax parameters /Adaptivity/Markers/BoundaryPreservedMarker

!syntax inputs /Adaptivity/Markers/BoundaryPreservedMarker

!syntax children /Adaptivity/Markers/BoundaryPreservedMarker
