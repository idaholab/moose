# NodalNormalsPreprocessor

!syntax description /UserObjects/NodalNormalsPreprocessor

The `NodalNormalsPreprocessor` populates the `nodal_normal_x`, `nodal_normal_y` and `nodal_normal_z`
variables with the local quadrature weight times the gradient of the shape function.
This object is created by the [NodalNormals action](AddNodalNormalsAction.md)
for the boundaries specified in the [!param](/Actions/AddNodalNormalsAction/boundary)
parameter. See the [`NodalNormals system`](syntax/NodalNormals/index.md) for more information.

!syntax parameters /UserObjects/NodalNormalsPreprocessor

!syntax inputs /UserObjects/NodalNormalsPreprocessor

!syntax children /UserObjects/NodalNormalsPreprocessor
