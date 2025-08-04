# SideValueSampler

!syntax description /VectorPostprocessors/SideValueSampler

The data output to CSV is output with the columns listed below. The declared vectors use the same
names.

- the id of the element that the quadrature point, on the side, belongs to.

- the values of the requested variables, with a column named after each variable.
  On internal sidesets, finite volume variables use an inverse distance weighted average to determine the face value.

- the X, Y, Z coordinates of the quadrature points on the side. In case of finite
  volume variables this is the cell centroid of the face.

!alert note title=General sampling
The `SideValueSampler` samples variables on the specified boundary on element side quadrature points. For more flexible sampling,
use the [PositionsFunctorValueSampler.md].

## Example input syntax

In this example, variable `u` and `v` are the solutions of two boundary value diffusion problems. Their value along the `top` boundary and along the `center` internal sidesets are reported using two `SideValueSampler`. The rows in the CSV output are sorted according the `x` coordinate along the boundary for the former, and the element `id` for the latter.

!listing test/tests/vectorpostprocessors/side_value_sampler/side_value_sampler.i block=VectorPostprocessors

!syntax parameters /VectorPostprocessors/SideValueSampler

!syntax inputs /VectorPostprocessors/SideValueSampler

!syntax children /VectorPostprocessors/SideValueSampler
