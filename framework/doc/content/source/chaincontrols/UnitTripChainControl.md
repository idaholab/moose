# UnitTripChainControl

This [ChainControl](syntax/ChainControls/index.md) produces a boolean trip
[chain control data](/ChainControlData.md), which is initially `false` but
upon being triggered, becomes `true` for the remainder of the simulation.
This trip value is named `<control>:tripped`, where `<control>` is the
user-given name of the `UnitTripChainControl`.
The trip is determined by the value of the input boolean value given by
[!param](/ChainControls/UnitTripChainControl/input) and
[!param](/ChainControls/UnitTripChainControl/trip_on_true). If `trip_on_true`
is set to `true`, then the trip occurs when the input value is `true`; else
it occurs when the input value is `false`.

!alert tip title=Converting to boolean control data
This object requires the input data to have type `bool`. If you have a data of
type `Real`, you can convert using [RealToBoolChainControl.md].

!syntax parameters /ChainControls/UnitTripChainControl

!syntax inputs /ChainControls/UnitTripChainControl

!syntax children /ChainControls/UnitTripChainControl
