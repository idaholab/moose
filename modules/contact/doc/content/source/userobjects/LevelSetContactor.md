# LevelSetContactor

## Description

Base class for a rigid contactor described implicitly by a signed-distance
(level-set) function.  Subclasses supply the pointwise signed distance and
outward normal; the base handles the contactor's translation state --- one
`Function` or coupled `Scalar` variable per Cartesian axis --- and returns
the transformed gap and normal to the downstream kernels.

The overall role of the contactor in the rigid-body contact formulation
is described on the [rigid-body contact theory
page](modules/contact/rigid_contact/theory.md#level-set-contactors).