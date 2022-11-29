# SideValueSampler

!syntax description /VectorPostprocessors/SideValueSampler

The data output to CSV is output with the columns listed below. The declared vectors use the same
names.

- the id of the element that the quadrature point, on the side, belongs to

- the values of the requested variables, with a column named after each variable

- the X, Y, Z coordinates of the quadrature points on the side

## Example input syntax

In this example, variable `u` and `v` are the solutions of two boundary value diffusion problems. Their value along the `top` boundary and along the `center` internal sidesets are reported using two `SideValueSampler`. The rows in the CSV output are sorted according the `x` coordinate along the boundary for the former, and the element `id` for the latter.

!listing test/tests/vectorpostprocessors/side_value_sampler/side_value_sampler.i block=VectorPostprocessors

!syntax parameters /VectorPostprocessors/SideValueSampler

!syntax inputs /VectorPostprocessors/SideValueSampler

!syntax children /VectorPostprocessors/SideValueSampler
