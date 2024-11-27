# TerminateChainControl

This [ChainControl](syntax/ChainControls/index.md) terminates the simulation
when a boolean [chain control data](/ChainControlData.md) has a given value.
The simulation may be terminated either by throwing an error or by signalling
to the [problem](/FEProblemBase.md) to terminate. This behavior is controlled
with the parameter [!param](/ChainControls/TerminateChainControl/throw_error).

!alert note title=The Terminator user object
An alternative way to terminate a simulation is to use a [Terminator.md] user object,
but `Terminator` works with [Postprocessors](Postprocessors/index.md) for the criterion
instead of a boolean chain control data.

!alert tip title=Converting to boolean control data
This object requires the input data to have type `bool`. If you have a data of
type `Real`, you can convert using [RealToBoolChainControl.md].

!syntax parameters /ChainControls/TerminateChainControl

!syntax inputs /ChainControls/TerminateChainControl

!syntax children /ChainControls/TerminateChainControl
