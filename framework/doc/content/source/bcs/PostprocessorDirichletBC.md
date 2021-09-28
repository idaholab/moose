# PostprocessorDirichletBC

!syntax description /BCs/PostprocessorDirichletBC

More information about Dirichlet boundary conditions and their mathematical meaning may be found in the
[DirichletBC.md] documentation.

A common postprocessor input for the `PostprocessorDirichletBC` is the [Receiver.md] postprocessor,
which is populated by a transfer from a [syntax/MultiApps/index.md]. The two simulations are then coupled through
this boundary condition, by setting the value of a variable on a boundary.

## Example Input Syntax

This example demonstrates the use case outlined above with `PostprocessorDirichletBC` on the `right` boundary
for variables `x` and `y`. The postprocessor input to the boundary conditions are `incoming_x` and `incoming_y`
respectively.

!listing test/tests/multiapps/centroid_multiapp/sub_app.i block=Postprocessors BCs

!syntax parameters /BCs/PostprocessorDirichletBC

!syntax inputs /BCs/PostprocessorDirichletBC

!syntax children /BCs/PostprocessorDirichletBC
