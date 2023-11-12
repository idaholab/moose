# LibtorchANNTrainer

!if function=hasLibtorch()
!syntax description /Trainers/LibtorchANNTrainer

## Overview

This trainer is dedicated to train a [utils/LibtorchArtificialNeuralNet.md]. For
a detailed description of the neural network trained by this object, visit
[utils/LibtorchArtificialNeuralNet.md]. The user can customize the neural network in the
trainer, however the optimization algorithm is hardcoded to be Adam.

## Example Input File Syntax

Let us try to approximate the following function: $y = \Pi_{i=1}^3|4x_i-2|$ over
the $[0,0.05]^3$ domain. For this, we select $125 (5 \times 5 \times 5)$ points using a tensor product grid
as follows:

!listing libtorch_nn/train.i block=Samplers

Following this, the function is evaluated using a vector postprocessor:

!listing libtorch_nn/train.i block=VectorPostprocessors

Once this is done, the corresponding inputs (from the sampler) and outputs
(from the postprocessor) are handed to the neural net trainer to optimize
the weights:

!listing libtorch_nn/train.i block=Trainers

!if! function=hasLibtorch()

We note that the user can set the architecture of the neural net using the
[!param](/Trainers/LibtorchANNTrainer/num_neurons_per_layer) and
[!param](/Trainers/LibtorchANNTrainer/activation_function) parameters.
The optimization algorithm depends on several parameters:
[!param](/Trainers/LibtorchANNTrainer/num_batches) defines how many batches the
training samples should be separated into, while [!param](/Trainers/LibtorchANNTrainer/num_epochs)
limits how many time we iterate over the batches.

The trained neural network can then be evaluated using [LibtorchANNSurrogate.md].

!syntax parameters /Trainers/LibtorchANNTrainer

!syntax inputs /Trainers/LibtorchANNTrainer

!syntax children /Trainers/LibtorchANNTrainer

!if-end!

!else
!include libtorch/libtorch_warning.md
