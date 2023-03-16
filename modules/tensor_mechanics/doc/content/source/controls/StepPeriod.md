# StepPeriod

The `StepPeriod` class derives from [ConditionalEnableControl](/ConditionalEnableControl.md).

`StepPeriod` objects are designed to control the active state of a MOOSE object,
based on whether the time value is in some range.

For example, consider a simulation that contains two [Kernels] "diff0" and "diff1". Initially,
"diff0" is active and after time 0.49 "diff1" becomes active and "diff0" is disabled. The following
code snippet demonstrates how this switching of kernels is achieved with the `TimePeriod` object.

This object, unlike [TimePeriod](/TimePeriod.md), is controlled by the loading 'step' concept. The user defines time periods,
i.e. steps, over which the boundary conditions and contraints are not disabled/enabled.
A step number is thus a required input to this control object. At the beginning of this step number,
boundary conditions and constraints are enabled or disabled, as directed by
[StepUserObject](/StepUserObject.md). Only one step is allowed per `StepPeriod` object.

!listing modules/tensor_mechanics/test/tests/umat/steps/elastic_temperature_steps_uo.i block=Controls/step1

!syntax parameters /Controls/StepPeriod

!syntax inputs /Controls/StepPeriod

!syntax children /Controls/StepPeriod
