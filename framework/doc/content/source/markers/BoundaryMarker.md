# BoundaryMarker

!syntax description /Adaptivity/Markers/BoundaryMarker

## Description

The `BoundaryMarker` is a stand-alone marker that marks all elements either
directly or within a given [!param](/Adaptivity/Markers/BoundaryMarker/distance)
to a boundary.

!alert warning title=Distributed Mesh
Currently this Marker may give inaccurate
results when using distributed meshes while also supplying the
[!param](/Adaptivity/Markers/BoundaryMarker/distance) parameter. Parts of the
boundary on different processors will not affect the distance calculation.
Marking elements directly adjacent to a boundary will always work.

!alert note title=Performance
This marker is not yet optimized for scalability
when using the [!param](/Adaptivity/Markers/BoundaryMarker/distance) parameter
and will have a complexity of order `O(N*M)`, where `N` is the total number of
elements in the mesh, and `M` is the number of elements adjacent to the
boundary.

## Example Input Syntax

!listing test/tests/markers/boundary_marker/adjacent.i block=Adaptivity

!listing test/tests/markers/boundary_marker/distance.i block=Adaptivity

!syntax parameters /Adaptivity/Markers/BoundaryMarker

!syntax inputs /Adaptivity/Markers/BoundaryMarker

!syntax children /Adaptivity/Markers/BoundaryMarker
