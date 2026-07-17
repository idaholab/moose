# MultiAppNearestNodeTransfer

!syntax description /Transfers/MultiAppNearestNodeTransfer

The projection mapping may be cached at the first execution of the transfer for better performance in later executions using the [!param](/Transfers/MultiAppNearestNodeTransfer/fixed_meshes) parameter. This is incompatible with adaptivity and refinement of the mesh.

!alert note
This transfer has been re-implemented to be more flexible. Please consider using [MultiAppGeneralFieldNearestLocationTransfer.md]

## Performance Considerations

For repeated transfers between meshes that do not move, adapt, or otherwise change, set
[!param](/Transfers/MultiAppNearestNodeTransfer/fixed_meshes) to `true`. This caches the
nearest-node mapping after the first execution and can greatly reduce the cost of subsequent
transfers. Do not use this option when either the source or target mesh changes, because the cached
nearest-node mapping can become invalid.

Restrict the source search domain when possible.
[!param](/Transfers/MultiAppNearestNodeTransfer/source_boundary) avoids searching unrelated source
nodes. The [!param](/Transfers/MultiAppNearestNodeTransfer/bbox_factor) parameter can be used when
the source bounding boxes must be inflated for a valid nearest-node search, but unnecessarily large
bounding boxes can increase the number of candidate source applications.
Separately, [!param](/Transfers/MultiAppNearestNodeTransfer/target_boundary) avoids assigning values
to unrelated target nodes.

## Example Input File Syntax

The following examples demonstrate the use the MultiAppNearestNodeTransfer for transferring data
to ([tosub]) and from ([fromsub]) sub-applications.

!listing multiapp_nearest_node_transfer/tosub_parent.i block=Transfers id=tosub caption=Example use of MultiAppNearestNodeTransfer for transferring data +to+ sub-applications.

!listing multiapp_nearest_node_transfer/fromsub_parent.i block=Transfers id=fromsub caption=Example use of MultiAppNearestNodeTransfer for transferring data +from+ sub-applications.

!syntax parameters /Transfers/MultiAppNearestNodeTransfer

!syntax inputs /Transfers/MultiAppNearestNodeTransfer

!syntax children /Transfers/MultiAppNearestNodeTransfer
