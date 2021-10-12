# CumulativeValuePostprocessor

!syntax description /Postprocessors/CumulativeValuePostprocessor

The running sum does not take into account tallies from failed time steps.
The `CumulativeValuePostprocessor` may be executed multiple times per time step
using the `execute_on` parameter.

!alert note
For time integration of postprocessors, use the [TimeIntegratedPostprocessor.md].

## Example input syntax

In this example, we tally the total number of nonlinear iterations over the time steps of the transient problem using a `CumulativeValuePostprocessor`.

!listing test/tests/postprocessors/cumulative_value_postprocessor/cumulative_value_postprocessor.i block=Postprocessors

!syntax parameters /Postprocessors/CumulativeValuePostprocessor

!syntax inputs /Postprocessors/CumulativeValuePostprocessor

!syntax children /Postprocessors/CumulativeValuePostprocessor
