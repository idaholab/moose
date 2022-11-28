# LineFunctionSampler

!syntax description /VectorPostprocessors/LineFunctionSampler

The output to CSV is by default ordered as follows:

- rows are ordered by point sampled along the line

- columns are ordered by alphabetical order of the functions sampled. The distance along the sampled line, and the x, y and z coordinates of the sampled points are added to the output as additional columns.


The order can be changed using the `sort_by` parameter.

!alert note title=Vector names / CSV output column names
`LineFunctionSampler` declares a vector for each spatial coordinate, (`x`, `y`, `z`), of the sampled points,
the distance along the sampled line in a vector called `id`,
and a vector named after each function sampled, containing the function values at each point.

## Example input syntax

In this example, the `LineFunctionSampler` is used to sample two functions across a line from '0 0 0' to '1 1 0'. The functions are specified directly in the `functions` parameter as their simple definition in terms of the x and y coordinates can be parsed directly. Functions are usually specified in the `[Functions]` block.

!listing test/tests/vectorpostprocessors/line_function_sampler/line_function_sampler.i block=VectorPostprocessors

!syntax parameters /VectorPostprocessors/LineFunctionSampler

!syntax inputs /VectorPostprocessors/LineFunctionSampler

!syntax children /VectorPostprocessors/LineFunctionSampler
