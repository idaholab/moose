# DRLRewardFunction

!syntax description /Functions/DRLRewardFunction

## Overview

Function describing the reward of for a Deep Reinforcement Learning algorithm in the form of:

!equation 
r = C_1 |x_{target}-x_{current}| + C_2

where $C_1$ and $C_2$ constants can be determined by the user. Furthermore, 
$x_{current}$ is a measured data, typically supplied by a postprocessor. 
For an example on how to use it in a DRL setting, see [LibtorchDRLControlTrainer.md].

!syntax parameters /Functions/DRLRewardFunction

!syntax inputs /Functions/DRLRewardFunction

!syntax children /Functions/DRLRewardFunction
