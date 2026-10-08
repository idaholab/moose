# ContactPairLowerDBlockGenerator

## Overview

This mesh generator automatically detects contact surface pairs among a list of
candidate boundaries and creates the lower-dimensional primary/secondary subdomain
blocks that [ContactAction](/actions/ContactAction.md) needs for mortar contact. It
is appended to the mesh generator tree by [ContactAction](/actions/ContactAction.md)
whenever `automatic_pairing_boundaries` is specified on the `[Contact]` action, so
it is not typically added directly by a user in the `[Mesh]` block.

Two candidate boundaries are considered a contact pair if they are within
`automatic_pairing_distance` of each other, using one of two detection strategies
selected with `automatic_pairing_method`:

- `NODE`: a KD-tree search over the nodes of the candidate boundaries, gathered from
  both their sidesets and nodesets. Two boundaries are paired if any of their nodes
  are within the pairing distance.
- `CENTROID`: the area-weighted centroid of each candidate sideset is computed, and
  two boundaries are paired if their centroids are within the pairing distance.

Each candidate boundary must be listed only once. Listing a boundary twice, either
repeated or both by name and by ID, is an error.

Within each pair, the boundary with the larger sideset area is assigned as the
primary surface and the other as the secondary surface. This places the Lagrange
multipliers on the smaller surface and reduces the number of secondary elements that
are only partially covered by the primary surface. When the two areas are equal to
within a relative tolerance, the boundary with the larger ID is chosen as primary.

Sideset areas and centroids are computed in Cartesian coordinates from the mesh
geometry alone, because the problem coordinate system is not available to a mesh
generator. For axisymmetric (`RZ`) or spherical (`RSPHERICAL`) problems, the areas
and centroids therefore differ from their true values in the problem coordinate
system, which can affect the primary/secondary assignment and the `CENTROID` pairing
distances.

For each detected pair, a primary and a secondary lower-dimensional subdomain block
are created (named using the `prefix` parameter), matching the naming convention
used internally by [ContactAction](/actions/ContactAction.md). If more than one
pair is found among the candidate boundaries, the generated block names are
suffixed with `_p_<primary name>_s_<secondary name>`, e.g. `_p_top_s_bottom` for
the pair (`top`, `bottom`), so that they remain unique.

Because the pairing search operates on the mesh's boundary and node information
directly, it requires a serial (non-distributed) mesh at the point this generator
runs.

!syntax parameters /Mesh/ContactPairLowerDBlockGenerator

!syntax inputs /Mesh/ContactPairLowerDBlockGenerator

!syntax children /Mesh/ContactPairLowerDBlockGenerator
