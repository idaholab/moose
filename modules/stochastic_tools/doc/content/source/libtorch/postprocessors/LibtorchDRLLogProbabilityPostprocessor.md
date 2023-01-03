# LibtorchDRLLogProbabilityPostprocessor

!if function=hasLibtorch()
!syntax description /Postprocessors/LibtorchDRLLogProbabilityPostprocessor

## Overview

This object is responsible for computing the logarithmic probability of an action taken by
a controller. It is used in a Deep Reinforcement Learning (DRL) context where we perturb the
actions using a normal distribution around the computed mean value. This helps with the random
exploration of the action space together with reducing the overfitting of the neural networks.

!if! function=hasLibtorch()

!syntax parameters /Postprocessors/LibtorchDRLLogProbabilityPostprocessor

!syntax inputs /Postprocessors/LibtorchDRLLogProbabilityPostprocessor

!syntax children /Postprocessors/LibtorchDRLLogProbabilityPostprocessor

!if-end!

!else
!include libtorch/libtorch_warning.md
