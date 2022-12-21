# LibtorchNeuralNetControl

!if! function=hasLibtorch()

!syntax description /Controls/LibtorchNeuralNetControl

## Overview

This object controls a physical process using a neural network. The neural network
can be either read from a file of transferred from a trainer. One can use the
[!param](/Controls/LibtorchNeuralNetControl/filename) parameter to load the neural network from file.
This capability supports two formats:

- Reading entire neural networks using a `TorchScript` format in which case the user needs to specify
  [!param](/Controls/LibtorchNeuralNetControl/torch_script_format) parameter;

- Reading neural network parameters from a binary (usually `.net`) file. In this case only parameters are read
  meaning that the structure of the neural network is not established in advance. For this reason, we need to specify the
  architecture using [!param](/Controls/LibtorchNeuralNetControl/num_neurons_per_layer) and
  [!param](/Controls/LibtorchNeuralNetControl/activation_function) parameters.


!syntax parameters /Controls/LibtorchNeuralNetControl

!syntax inputs /Controls/LibtorchNeuralNetControl

!syntax children /Controls/LibtorchNeuralNetControl

!if-end!

!else
This object controls a physical process using a neural network. The neural network
can be either read from a file of transferred from a trainer.

!include libtorch/libtorch_warning.md
