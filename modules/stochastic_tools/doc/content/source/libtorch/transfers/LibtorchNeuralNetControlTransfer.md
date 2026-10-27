# LibtorchNeuralNetControlTransfer

!if function=hasCapability('libtorch')
!syntax description /Transfers/LibtorchNeuralNetControlTransfer

## Overview

Transfer which copies a [LibtorchArtificialNeuralNet.md] from a neural net control trainer object
(say [LibtorchDRLControlTrainer.md]) on the main app to a [LibtorchNeuralNetControl](source/libtorch/controls/LibtorchNeuralNetControl.md) object on the sub app.
If the multiapp has several sub apps, the network is copied into the control of every sub app,
including when they are distributed over multiple processors.

!if! function=hasCapability('libtorch')

!syntax parameters /Transfers/LibtorchNeuralNetControlTransfer

!syntax inputs /Transfers/LibtorchNeuralNetControlTransfer

!syntax children /Transfers/LibtorchNeuralNetControlTransfer

!if-end!

!else
!include libtorch/libtorch_warning.md
