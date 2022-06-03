# MultiAppProjectionTransfer

!syntax description /Transfers/MultiAppProjectionTransfer

The projection mapping may be cached at the first execution of the transfer for better performance in later executions using the [!param](/Transfers/MultiAppProjectionTransfer/fixed_meshes) parameter. This is incompatible with adaptivity and refinement of the mesh.

## Example Input File Syntax

The following examples demonstrate the use the MultiAppProjectionTransfer for transferring data
to ([tosub]) and from ([fromsub]) sub-applications.

!listing multiapp_projection_transfer/tosub_parent.i block=Transfers id=tosub caption=Example use of MultiAppProjectionTransfer for transferring data +to+ sub-applications.

!listing multiapp_projection_transfer/fromsub_parent.i block=Transfers id=fromsub caption=Example use of MultiAppProjectionTransfer for transferring data +from+ sub-applications.

!syntax parameters /Transfers/MultiAppProjectionTransfer

!syntax inputs /Transfers/MultiAppProjectionTransfer

!syntax children /Transfers/MultiAppProjectionTransfer
