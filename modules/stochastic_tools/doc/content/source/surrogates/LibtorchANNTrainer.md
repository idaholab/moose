# LibtorchANNTrainer

!syntax description /Trainers/LibtorchANNTrainer

## Overview

This trainer is dedicated to generate a simple, feed-forward neural network.
The following description is meant tho summarize the basic concepts in the simplest
neural networks. For a more detailed introduction, we refer the reader to [!cite](muller1995neural).
The architecture of a simple feedforward neural network is presented below. The first layer
from the left the right are referred to as input and output layers,
while the layers between them are the hidden layers.

!media stochastic_tools/surrogates/libtorch_nn/neural_net.png style=width:65%; id=structure
      caption=The architecture of the simple feed-forward neural network in MOOSE-STM.


We see that the outputs ($\textbf{y}$) of the neural net can be expressed as function of the
inputs ($\textbf{x}$) and the corresponding model parameters (weights $w_{i,j}$, organized in
weight matrics $\textbf{W}$ and biases $b_i$ organized in the bias vector $\textbf{b}$)
in the following nested form:

!equation id=nn-explicit
\textbf{y} = \sigma(\textbf{W}^{(3)}\sigma(\textbf{W}^{(2)}\sigma(\textbf{W}^{(1)}\textbf{x}+\textbf{b}^{(1)})+\textbf{b}^{(2)})+\textbf{b}^{(3)}),

where $\sigma$ denotes the activation function. In this class, the activation function
in the hidden layers is set to RELU, while there is no activation function in the
output layer. It is apparent that the real functional dependence between the inputs and outputs
is approximated by the function in [nn-explicit]. As in most cases, the error in this approximation depends on the
smoothness of the function and the values of the model parameters. The weights and
biases in the function are determined by minimizing the error between the
approximate outputs of the neural net corresponding reference (training) values
over a training set. This optimization is carried out by an ADAM optimizer in
our case.

## Example Input File Syntax

Let us try to approximate the following function: $y = \Pi_{i=1}^3|4x_i-2|$ over
the $[0,0.05]^3$ domain. For this, we select 125 points using a tensor product grid
as follows:

!listing libtorch_nn/train.i block=Samplers

Following this, the function is evaluated using a vector postprocessor:

!listing libtorch_nn/train.i block=VectorPostprocessors

Once this is done, the corresponding inputs (from the sampler) and outputs
(from the postprocessor) are handed to the neural net trainer to optimize
the weights:

!listing libtorch_nn/train.i block=Trainers

We note that the user can set the architecture of the neural net using the
[!param](/Trainers/LibtorchANNTrainer/num_neurons_per_layer) parameter.
The optimization algorithm depends on several parameters.
[!param](/Trainers/LibtorchANNTrainer/num_batches) defines how many batches the
training samples should be separated into, while [!param](/Trainers/LibtorchANNTrainer/num_epochs)
limits how many time we iterate over the batches.

The trained neural network can then be evaluated using [LibtorchANNSurrogate.md].

!syntax parameters /Trainers/LibtorchANNTrainer

!syntax inputs /Trainers/LibtorchANNTrainer

!syntax children /Trainers/LibtorchANNTrainer
