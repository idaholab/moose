# PositionsFunctorValueSampler

!syntax description /VectorPostprocessors/PositionsFunctorValueSampler

!alert note
If the point value sampler is used with a functor that is discontinuous at the edge/face of a 2D/3D element, then the value from the element with the lowest ID will be returned, if and only if the user declared the functor as discontinuous using the [!param](/VectorPostprocessors/PositionsFunctorValueSampler/discontinuous) parameter.

The CSV output, with rows for each sampled point, contains the columns listed below. The vectors declared by `PositionsFunctorValueSampler`
share the same names as the column headers.

- the id of the elements containing the sampled points, with the convention mentioned above

- the values of the functor(s) requested

- the x, y and z coordinates of the requested sampled points

!syntax parameters /VectorPostprocessors/PositionsFunctorValueSampler

!syntax inputs /VectorPostprocessors/PositionsFunctorValueSampler

!syntax children /VectorPostprocessors/PositionsFunctorValueSampler
