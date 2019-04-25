# TimePeriod

The `TimePeriod` class derives from [ConditionalEnableControl](/ConditionalEnableControl.md).

`TimePeriod` objects are designed to control the active state of a MOOSE object,
based on whether the time value is in some range.

For example, consider a simulation that contains two [Kernels] "diff0" and "diff1". Initially,
"diff0" is active and after time 0.49 "diff1" becomes active and "diff0" is disabled. The following
code snippet demonstrates how this switching of kernels is achieved with the `TimePeriod` object.

!listing test/tests/controls/time_periods/kernels/kernels.i block=Controls

!syntax parameters /Controls/TimePeriod

!syntax inputs /Controls/TimePeriod

!syntax children /Controls/TimePeriod
