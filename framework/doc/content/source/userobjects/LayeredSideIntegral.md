# LayeredSideIntegral

!syntax description /UserObjects/LayeredSideIntegral

## How to define the layers

The parameters to define layers are explained in the [LayeredAverage.md] documentation.
The "block" parameter is no longer allowed to define the layers, unless the "boundary"
parameter is not set.

## How to retrieve the result

The result of a `LayeredSideIntegral` computation can be saved in an auxiliary variable using a
[SpatialUserObjectAux.md]. It can be output to a CSV file using a [SpatialUserObjectVectorPostprocessor.md].

## Additional computation options

Additional options for performing averages, interpolations and cumulative sums are explained in the
[LayeredAverage.md] documentation.

## Example input syntax

In this example, the integral of variable `u` is taken over the boundary `right` in the `y` direction over
three layers, on every linear iteration. The result of this averaging is stored in the variable
`layered_integral` using a [SpatialUserObjectAux.md] at the end of every time step, and output to a CSV file using a [SpatialUserObjectVectorPostprocessor.md].

!listing test/tests/userobjects/layered_side_integral/layered_side_integral.i block=UserObjects AuxKernels VectorPostprocessors

!syntax parameters /UserObjects/LayeredSideIntegral

!syntax inputs /UserObjects/LayeredSideIntegral

!syntax children /UserObjects/LayeredSideIntegral
