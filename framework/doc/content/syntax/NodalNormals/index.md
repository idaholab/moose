# NodalNormals System

The `NodalNormals` system is used to define the outward-facing normal of a boundary at a node.
The system is mostly used through the [AddNodalNormalsAction.md].

The nodal normals are stored in auxiliary Lagrange variables named `nodal_normal_x`/`y`/`z`.
They may be of first or second order to accommodate both mesh orders.

The nodal normals on boundaries are computed in two steps:
- first a [NodalNormalsPreprocessor.md] populates the `nodal_normal_x`/`y`/`z` variables with the local quadrature weight times the gradient of the shape function
- then a [NodalNormalsEvaluator.md] normalizes each component so that the nodal normal has a norm of 1

On corners, the first step is replaced by obtaining the normal from the [Assembly](source/base/Assembly.md).

!syntax list /NodalNormals objects=True actions=False subsystems=False

!syntax list /NodalNormals objects=False actions=False subsystems=True

!syntax list /NodalNormals objects=False actions=True subsystems=False
