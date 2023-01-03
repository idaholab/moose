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
!include libtorch/libtorch_warning.md
