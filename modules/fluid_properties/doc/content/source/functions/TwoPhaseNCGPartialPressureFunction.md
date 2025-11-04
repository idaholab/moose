# TwoPhaseNCGPartialPressureFunction

This function can be used to call a selection of property lookup functions from a
[TwoPhaseNCGPartialPressureFluidProperties.md] object. For a particular property
function specified by [!param](/Functions/TwoPhaseNCGPartialPressureFunction/property_call),
there is a certain number of input arguments, which are specified in the
parameters [!param](/Functions/TwoPhaseNCGPartialPressureFunction/arg1) and
[!param](/Functions/TwoPhaseNCGPartialPressureFunction/arg2) in the order
corresponding to the property call. Not all properties require the maximum
number of inputs, so the excess inputs should not be provided.

!syntax parameters /Functions/TwoPhaseNCGPartialPressureFunction

!syntax inputs /Functions/TwoPhaseNCGPartialPressureFunction

!syntax children /Functions/TwoPhaseNCGPartialPressureFunction
