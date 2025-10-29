# CSVPointValueSampler

!syntax description /VectorPostprocessors/CSVPointValueSampler

!alert note
If the csv point value sampler is used with a discontinuous variable on the edge/face of a 2D/3D element, then the value from the element with the lowest ID will be returned.

The CSV output, with rows for each sampled point, contains the columns listed below. The vectors declared by `CSVPointValueSampler`
share the same names as the column headers.

- the id of the sampled points

- the values of the variable(s) requested

- the x, y and z coordinates of the requested sampled points

The sampled points and the values are written to a csv file at every time step. The sorting order of the points can be changed using the sort_by parameter which takes x, y, z or id (increasing distance from start point) as input. 
Id can be supplied and/or sorted by the `point_id` from the csv file.

## Example input syntax

In this example, the variables `param1` and `param2` are sampled at the points read in from the csv file using a `CSVPointValueSampler`.

!listing test/tests/vectorpostprocessors/csv_point_value_sampler/csv_point_sampler.i block=VectorPostprocessors/point_sample

!syntax parameters /VectorPostprocessors/CSVPointValueSampler

!syntax inputs /VectorPostprocessors/CSVPointValueSampler

!syntax children /VectorPostprocessors/CSVPointValueSampler
