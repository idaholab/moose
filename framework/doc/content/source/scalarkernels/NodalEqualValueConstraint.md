# NodalEqualValueConstraint

!syntax description /ScalarKernels/NodalEqualValueConstraint

The constraint is imposed using a lagrange multiplier variable. The `NodalEqualValueConstraint` forms (part of) the equation for that variable, so its `variable` parameter is set to the lagrange multiplier scalar variable.

Constraining two nodes can be used to impose continuity conditions or to pin
two independent systems on disjoint meshes together. The limitation to two
nodes means that this kernel is not designed to work on sidesets or interfaces, except in 1D.
To impose an equal value constraint on a sideset, please prefer the
[EqualValueConstraint](source/constraints/EqualValueConstraint.md).

## Example input syntax

In this input file, a `NodalEqualValueConstraint` is used to impose an equality constraint between two disjoint line meshes.

!listing test/tests/mortar/1d/1d.i block=ScalarKernels

!syntax parameters /ScalarKernels/NodalEqualValueConstraint

!syntax inputs /ScalarKernels/NodalEqualValueConstraint

!syntax children /ScalarKernels/NodalEqualValueConstraint
