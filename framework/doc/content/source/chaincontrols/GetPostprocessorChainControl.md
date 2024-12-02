# GetPostprocessorChainControl

This [ChainControl](syntax/ChainControls/index.md) copies the current value of a
[Postprocessor](syntax/Postprocessors/index.md) into a [/ChainControlData.md] of
type `Real`. This data will be named as `<control>:value`, where `<control>`
is the user-given name of the `GetPostprocessorChainControl`, unless
[!param](/ChainControls/GetPostprocessorChainControl/name_data_same_as_postprocessor)
is set to `true`, in which case the data is named the same as the post-processor.

Note that modification
of this `ChainControlData` does not modify the value of the original post-processor.

!syntax parameters /ChainControls/GetPostprocessorChainControl

!syntax inputs /ChainControls/GetPostprocessorChainControl

!syntax children /ChainControls/GetPostprocessorChainControl
