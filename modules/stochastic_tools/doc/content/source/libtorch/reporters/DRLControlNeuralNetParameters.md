# DRLControlNeuralNetParameters

!if function=hasLibtorch()
!syntax description /Reporters/DRLControlNeuralNetParameters

## Overview

Converts the parameters of a [LibtorchArtificialNeuralNet.md] within a [LibtorchDRLControlTrainer.md]
into a vector which can be used for the visaluiation of the evolution of different parameters over the
training process.

## Example Syntax

!listing modules/stochastic_tools/test/tests/transfers/libtorch_nn_transfer/libtorch_drl_control_trainer.i block=Reporters

!if! function=hasLibtorch()

!syntax parameters /Reporters/DRLControlNeuralNetParameters

!syntax inputs /Reporters/DRLControlNeuralNetParameters

!syntax children /Reporters/DRLControlNeuralNetParameters

!if-end!

!else
!include libtorch/libtorch_warning.md
