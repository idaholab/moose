# NodalNormalsEvaluator

!syntax description /UserObjects/NodalNormalsEvaluator

The `NodalNormalsEvaluator` is added automatically by the [AddNodalNormalsAction.md].
This object is created for the boundaries specified in the [!param](/NodalNormals/AddNodalNormalsAction/boundary)
parameter. See the [syntax/NodalNormals/index.md] for more information.

The nodal normal is computed from the `nodal_normal_x`, `nodal_normal_y` and `nodal_normal_z`
variables, then it is used to divide the values for that node by the norm of the normal.

## Example input syntax

In this example, the `NodalNormals` system uses a `NodalNormalsEvaluator`, added behind the
scene by the action, to smooth the distance computation between two disjoint surfaces.

!listing test/tests/geomsearch/3d_moving_penetration_smoothing/pl_test3nns.i block=NodalNormals

!syntax parameters /UserObjects/NodalNormalsEvaluator

!syntax inputs /UserObjects/NodalNormalsEvaluator

!syntax children /UserObjects/NodalNormalsEvaluator
