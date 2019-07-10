# TriangleMesher

## Description

This mesh generator calls [Triangle](https://www.cs.cmu.edu/~quake/triangle.html) to triangulate a two-dimensional geometry and generate a two-dimensional unstructured mesh.
Triangle can be used only when libMesh is configured with `--disable-strict-lgpl`.
To describe a geometry, users first need to provide a list of spatial points through the parameter `points`.
These points are then used to form a set of segments, each specified with a starting point index and an ending point index.
It is noted that all indices are zero-based.
The segments can either be provided through parameter `segment` or be generated implicitly by the mesher.
If `segment` is not provided, the mesher will generate N segments, where N is the number of points, with pairs $(i, i+1), i=0,\cdots,N-2$ and $(N-1, 0)$, where the last segment wraps around the enclosure.
Users can decide how many intermediate points should be added on each segment with parameter `segment_subdivisions`.
Users can also give markers to these segments, which eventually become the boundary ID after mesh is generated.
It is noted marker 0 and marker 1 are reserved by Triangle, which are for internal and any boundaries not having assigned markers.
The segments forms regions bounded and separated by the segments that Triangle can detect.
Users can identify any region by giving an arbitrary interior point.
This allows users to assign subdomain IDs for any part of the geometry.
It is noted that 0 subdomain ID is reserved for regions that are not explicitly indicated by users in the input.
Also users can tell Triangle the maximum areas allowed for each individual region.
Users can create circular holes with three parameters `circular_hole_centers`, `circular_hole_radii` and `circular_hole_num_side_points`.
The mesher uses segments to approximate the holes, and points are distributed on the circles thus no exact volume preservation should be expected.
Users can also pass in other mesh generators through parameter `mesh_holes`.
The passed-in meshes must be two-dimensional.
Doing so will create holes from the passed-in meshes.
If users want the points of the hole boundary exactly match the boundary points of the passed-in meshes, they should set `allow_adding_points_on_boundary` to false, which essentially adds a switch `Y` to Triangle and prohibit generating any new points on boundary segments (including the hole boundary segments).

!syntax parameters /MeshGenerators/TriangleMesher

!syntax inputs /MeshGenerators/TriangleMesher

!syntax children /MeshGenerators/TriangleMesher
