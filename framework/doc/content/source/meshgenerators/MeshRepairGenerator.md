# MeshRepairGenerator

!syntax description /Mesh/MeshRepairGenerator

## Overview

The `MeshRepairGenerator` provides a collection of operations to repair defects / modify features in a mesh.
If the defects/features are not present in the mesh, the `MeshRepairGenerator` does not modify the mesh.
The operations currently implemented are:

- overlapping node merges. This operation considers all nodes in the entire mesh and looks for nodes in neighboring
  elements that may overlap. If overlaps are found, only one node is kept.

- flipping the orientation of negative volume elements. Negative volume elements can arise from a wrong orientation.

- separating elements in subdomains into several subdomains depending on their element types. Subdomains with a mix of
  element types are not supported by [Exodus.md] output. The new split subdomains' names have the type of the element appended to their respective names.

- merging boundaries with the same name but different boundary IDs.

- repairing sliver (near-degenerate) first-order 2D elements (`TRI3`, `QUAD4`, polygons). An element is flagged as a
  sliver if its area is below [!param](/Mesh/MeshRepairGenerator/sliver_triangle_area_fraction) times the mesh
  surface-area scale, or if every vertex other than the two ends of its longest edge lies within
  [!param](/Mesh/MeshRepairGenerator/sliver_triangle_flap_tol) times the longest-edge length from that edge. Either test
  can be disabled by setting its tolerance to 0. Each sliver is repaired by removing it and inserting its remaining
  vertices into the longest-edge neighbor's shared edge, dissolving the shared edge so the surface stays conformal (no
  holes or hanging nodes). The neighbor is promoted by its new vertex count: a triangle becomes a quadrilateral, and
  anything else becomes a polygon. If the longest edge is on a surface boundary (no neighbor to absorb the sliver), the
  sliver is left in place.

- renumbering the nodes and elements to have a contiguous ordering.

- splitting non-convex polygons into convex polygons

!syntax parameters /Mesh/MeshRepairGenerator

!syntax inputs /Mesh/MeshRepairGenerator

!syntax children /Mesh/MeshRepairGenerator
