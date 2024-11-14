# ChainControlDataPostprocessor

This [Postprocessor](syntax/Postprocessors/index.md) gets the current value of a
[/ChainControlData.md], which may be of type `Real` or `bool`. For `bool`,
a value of `true` converts to "1.0", and a value of `bool` converts to "0.0".

!alert warning title=A lag may be present
Within an execution step, [Controls](syntax/Controls/index.md) are executed
*before* post-processors, except for `INITIAL`, so depending on the sequence
of executions, a lag may be present.

!syntax parameters /Postprocessors/ChainControlDataPostprocessor

!syntax inputs /Postprocessors/ChainControlDataPostprocessor

!syntax children /Postprocessors/ChainControlDataPostprocessor
