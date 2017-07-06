# TimePeriod

The `TimePeriod` object is designed to control the active state of an object as a function
of time. Nearly all objects (e.g., [Kernels],
[BCs], etc.) may be enabled/disabled according to the simulation time using
the `TimePeriod` control.

For example, consider a simulation that contains two [Kernels] "diff0" and "diff1". Initially, "diff0" is active and after time 0.49 "diff1" becomes active and "diff0" is disable. The following code snippet demonstrates how this switching of kernels is achieve with
the `TimePeriod` object.

!listing test/tests/controls/time_periods/kernels/kernels.i block=Controls

Notice that the object names---the "enable_objects" and "disable_objects" parameters---expect a
list of object names, which are defined in two different ways. For a discussion on the naming
of objects and parameters see
[Object and Parameter Names](systems/Controls/index.md#object-and-parameter-names) section.

!syntax parameters /Controls/TimePeriod

!syntax inputs /Controls/TimePeriod

!syntax children /Controls/TimePeriod
