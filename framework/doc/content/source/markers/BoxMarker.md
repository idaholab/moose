# BoxMarker

!syntax description /Adaptivity/Markers/BoxMarker

## Description

The `BoxMarker` is a stand-alone marker that marks all
elements inside and outside for coarsening or refinement. The marker
operates using a bounding box that is specified by lower and upper
extents of the bounding box, in 3 dimensions regardless of the
dimensions of the problem.


## Example Input Syntax

!listing test/tests/markers/box_marker/box_marker_test.i block=Adaptivity

!syntax parameters /Adaptivity/Markers/BoxMarker

!syntax inputs /Adaptivity/Markers/BoxMarker

!syntax children /Adaptivity/Markers/BoxMarker
