# Assembly

The `Assembly` class holds most routines and objects related to assembling the numerical
system, such as the ones listed below. In the non-exhaustive list below, we refer to as
`local` the current element, face, quadrature points and neighbor values of each quantity.

- the current element

- the neighbor element

- the local volumes

- the Jacobian weights

- the local quadrature rules and points

- the local values of current variable, its spatial derivatives at first and second order,
  its curl on the element


It also contains these quantities for the mortar cases, on quadrature points.

This system is in charge of:

- reinitializing these quantities, through the `reinit...` methods,
  such as `reinit(Elem* elem)` for the current element, `reinitAtPhysical(...)` for a given vector of points
  or `reinitFVFace(face_info)` for the current face.

- getting the shape function values and its derivatives at the quadrature points, as well as the locations of the
  quadrature points from libmesh in the `buildFE...` routines for various locations.

- preparing the vectors for storing the local contributions to
  the Jacobian and the residual, by sizing and zeroing them, through the `prepare...` methods.

- caching contributions to the Jacobian and residual through the `cache...` methods.
  This can reduce the frequency of access to those global quantities, and is especially useful
  when using threads in the shared memory parallelism paradigm to be able to consider local
  contributions without locking the global vector.

- adding those local contributions to the global Jacobian and residual through the `add...` methods,
  for example `addResidual` for the local element residual or `addJacobianNeighbor` for the
  Jacobian on the neighbor element.
