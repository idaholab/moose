# Step 2a - Sampling values along a line

In this sidebar we'll go through how to sample variables along a line.
As noted previously in [Step 2](heat_conduction/tutorials/introduction/therm_step02.md),
this problem is expected to provide a solution for the temperature that
varies linearly as a function of the $x$ coordinate.

The variation of the solution as stored in the Exodus file can be visualized
along a line using tools like Paraview, but MOOSE also offers a way of directly
outputing values sampled along a line to a comma-separate value (CSV) file. 
Benefits of doing this include:

 - If a calculation is repeated many times, plotting of key quantities can
be automated.

 - The solution is sampled from the finite element interpolation used internally
by MOOSE, which provides the most accurate description of the solution.

 - The values sampled along the line could be used for other computations within
the MOOSE simulation.

To sample the values along a line, the MOOSE `VectorPostprocessor` system is used.
Whereas a `Postprocessor` in MOOSE computes a single value, a `VectorPostprocessor`
computes a vector of values. The `LineValueSampler` is a `VectorPostprocessor`
that computes values at a discrete set of points along a line.

!listing modules/heat_conduction/tutorials/introduction/therm_step02a.i

## Input file

### `VectorPostprocessors`

To add sampling along a line, a single block to define a `LineValueSampler`
is nested within the `VectorPostprocessors` top level block. Most of the parameters
are self-explanatory. They define the variable to be sampled, the start and
end points of the line, and the number of points. The order that the values
are output is controlled by the `sort_by` parameter, and in this case, they
are sorted by the `x` value since the line is oriented along the $x$ axis.
For lines that are not oriented along one of the Cartesian axes, the `sort_by = id`
option can be used to sort by the position along the line.

!listing modules/heat_conduction/tutorials/introduction/therm_step02a.i block=VectorPostprocessors

### `Outputs`

Adding the `LineValueSampler` block will result in values being computed,
but they will only exist internally in the code unless CSV output is requested
in the `Outputs` block. The simplest way to request this is to add `csv = true`,
similar to the `exodus = true` parameter. However, a block specific for the CSV
output is added to the `Outputs` block in this case to allow specifying parameters
to make the output more manageable. The `file_base` parameter permits controlling
the base name for the output file. By default a separate CSV file will be output
for each time step. To limit the amount of output, the `execute_on = final` parameter
makes the system only output a CSV file for the last time step.

!listing modules/heat_conduction/tutorials/introduction/therm_step02a.i block=Outputs

## Exercises

### Examining output

First, run the model as-is and examine the CSV file that is output. Try opening that
file in a plotting package to visualize the results.

### Output at multiple times

Try commenting out the `execute_on = final` line in the `csv` output block and
re-running the model. You should see multiple CSV files generated -- one for
each time step.

Once you've run this example we will move on to
[Step 3](heat_conduction/tutorials/introduction/therm_step03.md), where additional terms will be
added to the heat equation.
