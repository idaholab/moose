# RealToBoolChainControl

This [ChainControl](syntax/ChainControls/index.md) converts a `Real`-valued
[chain control data](/ChainControlData.md) to a `bool` data, with the name
`<control>:value`, where `<control>` is the user-given name of the `RealToBoolChainControl`:

- A value of 1 converts to `true`.
- A value of 0 converts to `false`.
- Other values result in an error.

Note that the tolerance on the value is 1e-12 in a double-precision MOOSE build.

!syntax parameters /ChainControls/RealToBoolChainControl

!syntax inputs /ChainControls/RealToBoolChainControl

!syntax children /ChainControls/RealToBoolChainControl
