# LoadSurrogateDataAction

This action operates on existing `SurrogateModel` objects contained within the `[Surrogates]` block.
If the model provides a filename (as shown below) the training data is initialized and the model is
ready for use via the evaluate method.

!listing load_store/evaluate.i block=Surrogates

!syntax parameters /Surrogates/AddSurrogateAction
