# SurfaceSubdomainsFromAllNormalsGenerator

!syntax description /Mesh/SurfaceSubdomainsFromAllNormalsGenerator

This mesh generator uses the flood algorithm defined in the [SubdomainsGeneratorBase.md] class and has
access to many of its options and parameters.

To generate contiguous subdomains, the [!param](/Mesh/SurfaceSubdomainsFromAllNormalsGenerator/contiguous_assignments_only)
parameter may be set to true. In this case, the flooding algorithm is called for each starting point
with a different subdomain ID.

It defines additionally the following techniques / heuristics:

- The subdomain to be assigned when flooding/painting from a surface element can be decided based on the subdomain IDs
  of a majority of its neighbors. This option is activated using the
  [!param](/Mesh/SurfaceSubdomainsFromAllNormalsGenerator/check_painted_neighbor_normals) parameter.
- A final sweep over all elements which re-assigns the subdomain id of the element based on the
  subdomain ID held by the majority of its neighbors. This heuristic is intended to remove isolated
  subdomain ID assignments. This option is activated with the
  [!param](/Mesh/SurfaceSubdomainsFromAllNormalsGenerator/select_max_neighbor_element_subdomains) parameter.


!syntax parameters /Mesh/SurfaceSubdomainsFromAllNormalsGenerator

!syntax inputs /Mesh/SurfaceSubdomainsFromAllNormalsGenerator

!syntax children /Mesh/SurfaceSubdomainsFromAllNormalsGenerator
