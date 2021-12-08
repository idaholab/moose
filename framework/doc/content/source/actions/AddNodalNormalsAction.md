# AddNodalNormalsAction

!syntax description /NodalNormals/AddNodalNormalsAction

Nodal normals can be used to compute the normals of a nodal variable in various locations.
They are specified as an object inside the `[NodalNormals]` block. They are composed of
a `NodalNormalsPreprocessor` and a `NodalNormalsEvaluator`, which this [MooseObjectAction.md]
adds to the [Problem](syntax/Problem/index.md).

This action adds and saves the nodal normals in the `nodal_normal_x`, `nodal_normal_y`
and `nodal_normal_z` auxiliary Lagrange nodal variables of the order specified with the
[!param](/NodalNormals/AddNodalNormalsAction/order) parameter.

More information about `NodalNormals` may be found on the
[NodalNormals syntax documentation](syntax/NodalNormals/index.md).

!syntax parameters /NodalNormals/AddNodalNormalsAction
