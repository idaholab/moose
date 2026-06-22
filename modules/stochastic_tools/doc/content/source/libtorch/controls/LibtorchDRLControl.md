# LibtorchDRLControl

!if function=hasCapability('libtorch')
!syntax description /Controls/LibtorchDRLControl

## Overview

This object is the runtime policy executor paired with
[LibtorchDRLControlTrainer](source/libtorch/trainers/LibtorchDRLControlTrainer.md).
It extends
[LibtorchNeuralNetControl](source/libtorch/controls/LibtorchNeuralNetControl.md)
with stochastic policy sampling, action reuse, optional smoothing, and
restartable policy state. For deterministic execution of the same actor, set
[!param](/Controls/LibtorchDRLControl/stochastic) to `false`. Use
[LibtorchNeuralNetControl](source/libtorch/controls/LibtorchNeuralNetControl.md)
instead when a plain deterministic neural-net control object is preferred
without the DRL-specific execution features.

## Execution Model

`LibtorchDRLControl` only supports `TIMESTEP_BEGIN`. On each execution it reads
the current observation values, combines them with the stored history implied by
[!param](/Controls/LibtorchDRLControl/input_timesteps), and evaluates the actor
when a new policy action is needed.

If [!param](/Controls/LibtorchDRLControl/stochastic) is `true`, the action is
sampled from the actor distribution and the corresponding log probabilities are
stored for PPO training. If it is `false`, the deterministic actor output is
used instead. The policy evaluation can be reused across multiple controller
executions with [!param](/Controls/LibtorchDRLControl/num_steps_in_period), so a
new action is only sampled every configured number of executions.

The applied control can also be relaxed with
[!param](/Controls/LibtorchDRLControl/smoother):
\begin{equation}
u_t^{\mathrm{applied}} = u_{t-1}^{\mathrm{applied}} +
\alpha\left(u_t^{\mathrm{policy}} - u_{t-1}^{\mathrm{applied}}\right),
\end{equation}
where $\alpha$ is the `smoother` value. Setting `smoother = 1` applies the raw
policy action directly.

The controller stores the observation history, smoothed signal, and the libtorch
CPU random-number-generator state as restartable data. This keeps stochastic
recovered runs aligned with uninterrupted runs, provided the same controller
state is recovered.

!if! function=hasCapability('libtorch')

!syntax parameters /Controls/LibtorchDRLControl

!syntax inputs /Controls/LibtorchDRLControl

!syntax children /Controls/LibtorchDRLControl

!if-end!

!else
!include libtorch/libtorch_warning.md
