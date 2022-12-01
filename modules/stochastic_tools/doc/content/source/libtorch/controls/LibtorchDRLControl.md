# LibtorchDRLControl

!if function=hasLibtorch()
!syntax description /Controls/LibtorchDRLControl

## Overview

This object controls a physical process using a neural network, just like [LibtorchNeuralNetControl](source/libtorch/controls/LibtorchNeuralNetControl.md),
with an additional functionality of randomizing the action values to avoid overfitting in the control process.
This control object is supposed to be used in conjunction with [LibtorchDRLControlTrainer.md]. In other
cases when the neural network needs to be simply evaluated, the user is encouraged to use [LibtorchNeuralNetControl](source/libtorch/controls/LibtorchNeuralNetControl.md).

!if! function=hasLibtorch()

!syntax parameters /Controls/LibtorchDRLControl

!syntax inputs /Controls/LibtorchDRLControl

!syntax children /Controls/LibtorchDRLControl

!if-end!

!else
!alert warning
The detailed documentation of this object is only available when Moose is compiled with Libtorch.
For instructions on how to compile Moose with Libtorch, click [here](install_libtorch.md).
