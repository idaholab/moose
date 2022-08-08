# DRLControlNeuralNetParameters

Converts the parameters of a [LibtorchArtificialNeuralNetwork.md] within a [LibtorchDRLControlTrainer.md]
into a vector which can be used for the visaluiation of the evolution of different parameters over the
training process.

## Example Syntax

!listing modules/stochastic_tools/test/tests/transfers/libtorch_nn_transfer/libtorch_drl_control_trainer.i block=VectorPostprocessors

!syntax parameters /VectorPostprocessors/DRLControlNeuralNetParameters

!syntax inputs /VectorPostprocessors/DRLControlNeuralNetParameters

!syntax children /VectorPostprocessors/DRLControlNeuralNetParameters
