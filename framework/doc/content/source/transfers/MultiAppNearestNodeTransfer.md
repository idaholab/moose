# MultiAppNearestNodeTransfer

!syntax description /Transfers/MultiAppNearestNodeTransfer

The projection mapping may be cached at the first execution of the transfer for better performance in later executions using the [!param](/Transfers/MultiAppNearestNodeTransfer/fixed_meshes) parameter. This is incompatible with adaptivity and refinement of the mesh.

!alert note
This transfer has been re-implemented to be more flexible. Please consider using [MultiAppGeneralFieldNearestNodeTransfer.md]

## Example Input File Syntax

The following examples demonstrate the use the MultiAppNearestNodeTransfer for transferring data
to ([tosub]) and from ([fromsub]) sub-applications.

!listing multiapp_nearest_node_transfer/tosub_parent.i block=Transfers id=tosub caption=Example use of MultiAppNearestNodeTransfer for transferring data +to+ sub-applications.

!listing multiapp_nearest_node_transfer/fromsub_parent.i block=Transfers id=fromsub caption=Example use of MultiAppNearestNodeTransfer for transferring data +from+ sub-applications.

!syntax parameters /Transfers/MultiAppNearestNodeTransfer

!syntax inputs /Transfers/MultiAppNearestNodeTransfer

!syntax children /Transfers/MultiAppNearestNodeTransfer
