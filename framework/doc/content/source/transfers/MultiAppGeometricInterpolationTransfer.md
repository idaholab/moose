# MultiAppGeometricInterpolationTransfer

The `MultiAppGeometricInterpolationTransfer` transfers the nearest node's source variables to the nearest node on the
target mesh using mesh interpolation, including the ability to utilize the displaced
configuration for either or both the source and target.  
The `MultiAppGeometricInterpolationTransfer` also offers extrapolation on non-overlapping domains where the target data
can be computed from source nodes not inside the target mesh.
Other interpolation transfers like the [MultiAppShapeEvaluationTransfer.md] are not able to extrapolate data and will only work for fully overlapping domains.

Nodal transfers using the default settings for this interpolation scheme, `interp_type=inverse_distance`
and `num_points=3`, will find the three closest points on the source mesh to a node on the target mesh.  
The source data from the three closest nodes will then be interpolated to the target node using [inverse distance weighting](https://en.wikipedia.org/wiki/Inverse_distance_weighting).   
Inverse distance interpolation is best suited for the interpolation of point cloud data in the source mesh onto a target mesh.  

!alert note
The [MultiAppShapeEvaluationTransfer.md] may be a better choice for nodal transfers
between two meshes with fully overlapping domains because the element shape functions will be used in the transfer.
However, as mentioned above, only the `MultiAppGeometricInterpolationTransfer` can be used to extrapolate data between meshes on domains that do not fully overlap.

Using `MultiAppGeometricInterpolationTransfer` with the default interpolation settings for mesh to mesh nodal transfers of data
is not deterministic when the point being interpolated to on the
target mesh is exactly the same distance away from more than `num_points` on the source mesh.  
This nondeterministic behavior in the `MultiAppGeometricInterpolationTransfer` node-to-node transfer
can lead to different data being transferred when the numerics
of a problem are changed by something like the parallel decomposition or compiler settings.  

This inconsistency can occur on structured meshes when the target mesh is a refined version of the source mesh.  
In this scenario, a target node will be placed equidistant from several source nodes.  
For a 2D structured mesh of quadrilaterals, the refined mesh's target node would be equidistant from 4 nodes on the source mesh.  
With the default `num_points=3`, the three points chosen from the four equidistant
nodes on the source mesh will be arbitrary and has been shown to be dependent on
the parallel decomposition of the mesh.  
This problem can be made deterministic by increasing `num_points=4` so that
all of the nodes in the element are used for the interpolation.


## Example Syntax

!listing test/tests/transfers/multiapp_interpolation_transfer/fromsub_parent.i start=[Transfers] end=elemental_fromsub footer=[]

!syntax parameters /Transfers/MultiAppGeometricInterpolationTransfer

!syntax inputs /Transfers/MultiAppGeometricInterpolationTransfer

!syntax children /Transfers/MultiAppGeometricInterpolationTransfer
