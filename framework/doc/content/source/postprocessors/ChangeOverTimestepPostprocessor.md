# ChangeOverTimestepPostprocessor

!syntax description /Postprocessors/ChangeOverTimestepPostprocessor

The `ChangeOverTimestepPostprocessor` can be used to track convergence of additional quantities computed by postprocessors. For example, in conjugate heat transfer simulations, the total energy stored in the fluid should be tracked as an additional convergence metric, and not just the residual from the equations.

The 'INITIAL' `execute_on` flag should be included in the tracked postprocessor `execute_on` parameter if the `ChangeOverTimestepPostprocessor` is set to compute the difference with regards to the initial value.

## Example input syntax

In this example, the `ChangeOverTimestepPostprocessor` is set to track the change of a `FunctionValuePostprocessor`, as a test of correctness. The `FunctionValuePostprocessor`'s evolution is known as it is defined by a `ParsedFunction`.

!listing test/tests/postprocessors/change_over_time/change_over_time.i block=Postprocessors Functions
!syntax parameters /Postprocessors/ChangeOverTimestepPostprocessor

!syntax inputs /Postprocessors/ChangeOverTimestepPostprocessor

!syntax children /Postprocessors/ChangeOverTimestepPostprocessor
