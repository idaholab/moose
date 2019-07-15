# Indicators System

The Indicator system is a sub-system of the [syntax/Adaptivity/index.md]. The `Indicator` objects
serve to provide a measurement of "error" that are used to inform the
[syntax/Adaptivity/Markers/index.md]. When creating a custom `Indicator` object the child object
must override the `computeIndicator` method.

Allthough created for the adaptivity system, indicators can be used stand-alone for computing error
values, since the values computed are simply auxiliary variables, see [AuxVariables/index.md].


!syntax list /Adaptivity/Indicators objects=True actions=False subsystems=False

!syntax list /Adaptivity/Indicators objects=False actions=False subsystems=True

!syntax list /Adaptivity/Indicators objects=False actions=True subsystems=False
