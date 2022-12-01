# LibtorchControlValuePostprocessor

!if function=hasLibtorch()
!syntax description /Postprocessors/LibtorchControlValuePostprocessor

## Overview

This object is responsible for querying the action values taken by a neural network
within a [LibtorchNeuralNetControl](source/libtorch/controls/LibtorchNeuralNetControl.md) object.

!if! function=hasLibtorch()

!syntax parameters /Postprocessors/LibtorchControlValuePostprocessor

!syntax inputs /Postprocessors/LibtorchControlValuePostprocessor

!syntax children /Postprocessors/LibtorchControlValuePostprocessor

!if-end!

!else
!alert warning
The detailed documentation of this object is only available when Moose is compiled with Libtorch.
For instructions on how to compile Moose with Libtorch, click [here](install_libtorch.md).
