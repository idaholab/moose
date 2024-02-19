# LibtorchDRLControlTrainer

!if function=hasLibtorch()
!syntax description /Trainers/LibtorchDRLControlTrainer

## Overview

This object is supposed to train a Deep Reinforcement Learning (DRL) controller
using the Proximal Policy Optimization (PPO) algorithm [!cite](schulman2017proximal).

## Example Input File Syntax

!if! function=hasLibtorch()

!syntax parameters /Trainers/LibtorchDRLControlTrainer

!syntax inputs /Trainers/LibtorchDRLControlTrainer

!syntax children /Trainers/LibtorchDRLControlTrainer

!if-end!

!else
!include libtorch/libtorch_warning.md
