# ElementGroupCentroidPositions

!syntax description /Positions/ElementGroupCentroidPositions

## Overview

This [Positions](syntax/Positions/index.md) object can be used to keep track of the centroids of
assemblies, fuel pins, or axial levels of fuel pins, using the `assembly_id`, `pin_id` and `plane_id`
respectively in the Reactor module.

Each block or extra element id specified in the parameters creates new
bins; it does not indicate a wider group of element to compute the centroid of.

So for example, if three blocks and two types of extra element ids are specified,
and for the latter two and four specific ids are specified, then the centroids will be computed for
each of the `3 * 2 * 4` groups of elements.

!alert note
If any group of element is empty, the `ElementGroupCentroidPositions` will emit a warning
and automatically remove it from the positions calculated.

!alert note
This object does not support distributed meshes. If you require its use with distributed meshes
use a replicated mesh to generate the positions then after converting them to the expected format,
load them from file using a [FilePositions.md].
If you are also using displaced meshes, then contact a MOOSE developer.

## Example input syntax

In this example, we first compute the `Positions` of the centroids of each subdomain in the mesh.

!listing tests/positions/element_group_centroid_positions.i block=Positions/all_mesh_blocks

We also show we can narrow this down to a single group

!listing tests/positions/element_group_centroid_positions.i block=Positions/block_1

We then introduce extra element integer ids. Every position will correspond to the centroid of
elements that are in a given subdomain, and also hold the extra element integer ids specified.
Every additional element integer creates more combinations, more groups of elements for which the
centroids are computed and stored.

!listing tests/positions/element_group_centroid_positions.i block=Positions/block_and_one_id Positions/block_and_two_id

With this example, we show that specifying a simple space, like this `; ;` will request all
the available values for the element ids. This is easier than specifying them all in the input.

!listing tests/positions/element_group_centroid_positions.i block=Positions/block_and_three_id

Finally, we do not use the `block` parameter to generate the centroids of the groups
corresponding to combinations of three extra element ids.

!listing tests/positions/element_group_centroid_positions.i block=Positions/three_ids

!syntax parameters /Positions/ElementGroupCentroidPositions

!syntax inputs /Positions/ElementGroupCentroidPositions

!syntax children /Positions/ElementGroupCentroidPositions
