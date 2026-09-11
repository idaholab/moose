# ContactPairLowerDBlockGenerator

!syntax description /Mesh/ContactPairLowerDBlockGenerator

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

- `NODE`: a KD-tree search over the nodes of the candidate boundaries. Two
  boundaries are paired if any of their nodes are within the pairing distance.
- `CENTROID`: the center of gravity of each candidate boundary is computed, and two
  boundaries are paired if their centroids are within the pairing distance.

For each detected pair, a primary and a secondary lower-dimensional subdomain block
are created (named using the `prefix` parameter), matching the naming convention
used internally by [ContactAction](/actions/ContactAction.md). If more than one
pair is found among the candidate boundaries, the generated block names are
suffixed with the boundary names of that pair so that they remain unique.

Because the pairing search operates on the mesh's boundary and node information
directly, it requires a serial (non-distributed) mesh at the point this generator
runs.

!syntax parameters /Mesh/ContactPairLowerDBlockGenerator

!syntax inputs /Mesh/ContactPairLowerDBlockGenerator

!syntax children /Mesh/ContactPairLowerDBlockGenerator
