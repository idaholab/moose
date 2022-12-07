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
!include libtorch/libtorch_warning.md
