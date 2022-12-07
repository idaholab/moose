# LibtorchArtificialNeuralNetParameters

!if function=hasLibtorch()
!syntax description /Reporters/LibtorchArtificialNeuralNetParameters

## Overview

Converts the parameters of a [LibtorchArtificialNeuralNet.md] within a
[LibtorchNeuralNetControl](source/libtorch/controls/LibtorchNeuralNetControl.md)
into a vector which can be used for the visalization of the evolution of different parameters over the
simulation.

## Example Syntax

!listing test/tests/controls/libtorch_nn_control/read_control.i block=Reporters

!if! function=hasLibtorch()

!syntax parameters /Reporters/LibtorchArtificialNeuralNetParameters

!syntax inputs /Reporters/LibtorchArtificialNeuralNetParameters

!syntax children /Reporters/LibtorchArtificialNeuralNetParameters

!if-end!

!else
!include libtorch/libtorch_warning.md
