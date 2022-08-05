# LibtorchNeuralNetControl

!syntax description /Controls/LibtorchNeuralNetControl

## Overview

This object can be used to control a process with multiple control parameters determined using a neural network.
The neural network can be either trained in a trainer such as the Deep Reinforcement Learning based trainer in the
Stochastic tools Module or read from a file in case it was trained previously in MOOSE or in python.

## Example usage

The following example show how to use a neural network, trained by an external application, for the control
of a transient diffusion problem. The kernels describing the problem are defined below:

!listing controls/libtorch_nn_control/read_torchscript_control.i block=Kernels

We would like to control the value of `anti_source` based on tha maximum temperature of the system using a neural network.
For this we first define a `Postprocessor` which can determine the maximum temperature:

!listing controls/libtorch_nn_control/read_torchscript_control.i block=Postprocessors

The `control_value` postprocessor is responsible for storing the value of the control signal for visualization purposes.
As a next step, we create a `Control` object which reads a neural network from a torch-script file:

!listing controls/libtorch_nn_control/read_torchscript_control.i block=Controls

Note that there are two options to read a neural network. If the
[!param](/Controls/LibtorchNeuralNetControl/torch_script_format) is set to `true`, the user
has no control over the architecture of the neural network, it is encoded in the torch-script file. The other option is
to parse the parameter values from a binary file output. In this case the user needs to define the exact same net
architecture as was used for the training. This can be done using
[!param](/Controls/LibtorchNeuralNetControl/num_neurons_per_layer) parameter.

Furthermore, the physical inputs of the neural net can be shifted and scaled if needed using
[!param](/Controls/LibtorchNeuralNetControl/response_shift_factors) and
[!param](/Controls/LibtorchNeuralNetControl/response_scaling_factors). This is due to the fact that
the standardization of inputs is a common practice in the training of neural networks. Lastly, the
outputs of the neural net can be scaled too using [!param](/Controls/LibtorchNeuralNetControl/action_scaling_factors).

For the full input file, see:

!listing controls/libtorch_nn_control/read_torchscript_control.i

!syntax parameters /Controls/LibtorchNeuralNetControl

!syntax inputs /Controls/LibtorchNeuralNetControl

!syntax children /Controls/LibtorchNeuralNetControl
