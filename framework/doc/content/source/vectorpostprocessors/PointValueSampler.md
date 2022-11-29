# PointValueSampler

!syntax description /VectorPostprocessors/PointValueSampler

!alert note
If the point value sampler is used with a discontinuous variable on the edge/face of a 2D/3D element, then the value from the element with the lowest ID will be returned.

The CSV output, with rows for each sampled point, contains the columns listed below. The vectors declared by `PointValueSampler`
share the same names as the column headers.

- the id of the elements containing the sampled points, with the convention mentioned above

- the values of the variable(s) requested

- the x, y and z coordinates of the requested sampled points


## Example input syntax

In this example, the variables `u` and `v` are sampled at three points, `(0.1 0.1 0)`, `(0.23 0.4 0)` and `(0.78 0.2 0)` using a `PointValueSampler`.

!listing test/tests/vectorpostprocessors/point_value_sampler/point_value_sampler.i block=VectorPostprocessors

!syntax parameters /VectorPostprocessors/PointValueSampler

!syntax inputs /VectorPostprocessors/PointValueSampler

!syntax children /VectorPostprocessors/PointValueSampler
