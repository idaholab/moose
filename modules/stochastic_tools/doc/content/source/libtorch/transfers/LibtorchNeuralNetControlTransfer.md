# LibtorchNeuralNetControlTransfer

!if function=hasLibtorch()
!syntax description /Transfers/LibtorchNeuralNetControlTransfer

## Overview

Transfer which copies a [LibtorchArtificialNeuralNet.md] from a neural net control trainer object
(say [LibtorchDRLControlTrainer.md]) on the main app to a [LibtorchNeuralNetControl](source/libtorch/controls/LibtorchNeuralNetControl.md) object on the sub app.

!if! function=hasLibtorch()

!syntax parameters /Transfers/LibtorchNeuralNetControlTransfer

!syntax inputs /Transfers/LibtorchNeuralNetControlTransfer

!syntax children /Transfers/LibtorchNeuralNetControlTransfer

!if-end!

!else
!include libtorch/libtorch_warning.md
