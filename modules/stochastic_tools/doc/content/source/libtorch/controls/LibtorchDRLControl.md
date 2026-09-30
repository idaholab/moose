# LibtorchDRLControl

!if! function=hasCapability('libtorch')

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
used instead. The policy evaluation can be reused across multiple time steps
with [!param](/Controls/LibtorchDRLControl/num_steps_in_period). For
`num_steps_in_period = P`, the actor is evaluated on steps
$1, P+1, 2P+1, \ldots$, and the action is held in between. A transfer that
executes the control before the first time step also triggers an evaluation at
step 0. The observation history only advances when the actor is evaluated, so
consecutive entries of the stacked input are one control period apart. Set
[!param](/Trainers/LibtorchDRLControlTrainer/timestep_window) on the trainer to
the same value so that it rebuilds the observations the controller used.

The applied control can also be smoothed exponentially with
[!param](/Controls/LibtorchDRLControl/control_smoothing_factor):
\begin{equation}
u_t^{\mathrm{applied}} = u_{t-1}^{\mathrm{applied}} +
\alpha\left(u_t^{\mathrm{policy}} - u_{t-1}^{\mathrm{applied}}\right),
\end{equation}
where $\alpha \in (0, 1]$ is the `control_smoothing_factor` value. Setting
`control_smoothing_factor = 1` applies the raw policy action directly. Because
each applied value lies between the previous applied value and the policy action,
smoothing does not overshoot the action or leave the range set by
[!param](/Controls/LibtorchDRLControl/min_control_value) and
[!param](/Controls/LibtorchDRLControl/max_control_value), provided the starting
value lies in that range. The smoothed signal starts from zero, so with $\alpha < 1$
the first applied values lie between zero and the policy action. The smoothing is
applied every time step, including the steps within a control period that reuse
the previous action. [LibtorchControlValuePostprocessor.md] reports the policy
action $u_t^{\mathrm{policy}}$, not the applied value.

The controller stores the observation history, smoothed signal, and the libtorch
CPU random-number-generator state as restartable data. This keeps stochastic
recovered runs aligned with uninterrupted runs, provided the same controller
state is recovered.

!syntax parameters /Controls/LibtorchDRLControl

!syntax inputs /Controls/LibtorchDRLControl

!syntax children /Controls/LibtorchDRLControl

!if-end!

!else
!include libtorch/libtorch_warning.md
