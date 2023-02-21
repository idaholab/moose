# LayeredIntegral

!syntax description /UserObjects/LayeredIntegral

## How to define the layers

The parameters to define layers are explained in the [LayeredAverage.md] documentation.

## How to retrieve the result

The result of a `LayeredIntegral` computation can be saved in an auxiliary variable using a
[SpatialUserObjectAux.md]. It can be output to a CSV file using a [SpatialUserObjectVectorPostprocessor.md].

## Additional computation options

Additional options for performing averages, interpolations and cumulative sums are explained in the
[LayeredAverage.md] documentation.

## Example input syntax

In this example, the integral of variable `u` is taken over the whole domain in direction `y` over
three layers, on every linear iteration. The result of this averaging is stored in the variable
`layered_integral` using a [SpatialUserObjectAux.md] at the end of every timestep, and output to a
CSV file using a [SpatialUserObjectVectorPostprocessor.md].

!listing test/tests/userobjects/layered_integral/layered_integral_test.i block=UserObjects AuxKernels VectorPostprocessors

!syntax parameters /UserObjects/LayeredIntegral

!syntax inputs /UserObjects/LayeredIntegral

!syntax children /UserObjects/LayeredIntegral
