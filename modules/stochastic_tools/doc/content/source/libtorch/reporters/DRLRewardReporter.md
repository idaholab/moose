# DRLRewardReporter

!if function=hasLibtorch()
!syntax description /Reporters/DRLRewardReporter

## Overview

This reporter collects the reward values from the Deep Reinforcement Learning (DRL) trainers
so that the training process can be easily visualized.

!if! function=hasLibtorch()

!syntax parameters /Reporters/DRLRewardReporter

!syntax inputs /Reporters/DRLRewardReporter

!syntax children /Reporters/DRLRewardReporter

!if-end!

!else
!alert warning
The detailed documentation of this object is only available when Moose is compiled with Libtorch.
For instructions on how to compile Moose with Libtorch, click [here](install_libtorch.md).
