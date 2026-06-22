# LibtorchDRLControlTrainer

!if function=hasCapability('libtorch')
!syntax description /Trainers/LibtorchDRLControlTrainer

## Overview

This object trains an on-policy actor-critic controller using Proximal Policy
Optimization (PPO) [!cite](schulman2017proximal) together with generalized
advantage estimation (GAE) [!cite](schulman2015gae). Reporter trajectories are
assembled into transitions, flattened across samples, shuffled into mini-batches,
and then used to update separate actor and critic neural networks.

## Algorithm Summary

For `input_timesteps = H`, the observation passed to the actor and critic is the
stacked history
\begin{equation}
o_t = \left[s_t,\ s_{t-1},\ \ldots,\ s_{t-H+1}\right],
\end{equation}
where early missing history entries are filled with the earliest available
reporter value.

Each collected trajectory is converted into the tuples
$(o_t, a_t, \log \pi_{\mathrm{old}}(a_t|o_t), r_t, o_{t+1})$. The
[!param](/Trainers/LibtorchDRLControlTrainer/shift_outputs) option aligns the
reported action and log-probability sequences with the reward sequence when the
control is applied at the beginning of a time step and the reward is measured at
the end of that step. The
[!param](/Trainers/LibtorchDRLControlTrainer/timestep_window) option can be used
to downsample long reporter trajectories before these transitions are assembled.

The critic is first evaluated on $o_t$ and $o_{t+1}$, and the temporal-difference
residual is computed as
\begin{equation}
\delta_t = r_t + \gamma V_{\phi}(o_{t+1}) - V_{\phi}(o_t).
\end{equation}
The trainer then computes the reverse-time GAE recursion
\begin{equation}
\hat{A}_t = \delta_t + \gamma \lambda \hat{A}_{t+1},
\end{equation}
with critic regression targets
\begin{equation}
\hat{V}_t = \hat{A}_t + V_{\phi}(o_t).
\end{equation}

For each mini-batch, PPO uses the probability ratio
\begin{equation}
r_t(\theta) = \exp\left(\log \pi_{\theta}(a_t|o_t) -
\log \pi_{\mathrm{old}}(a_t|o_t)\right),
\end{equation}
and the actor objective implemented here is
\begin{equation}
L_{\mathrm{actor}} =
-\frac{1}{N}\sum_t \left[
\min\left(r_t(\theta)\hat{A}_t,
\mathrm{clip}\left(r_t(\theta), 1-\epsilon, 1+\epsilon\right)\hat{A}_t\right)
+ c_H \mathcal{H}\left[\pi_{\theta}(\cdot|o_t)\right]
\right].
\end{equation}
The critic is trained with a mean-squared-error loss,
\begin{equation}
L_{\mathrm{critic}} = \frac{1}{N}\sum_t
\left(V_{\phi}(o_t) - \hat{V}_t\right)^2.
\end{equation}

If [!param](/Trainers/LibtorchDRLControlTrainer/standardize_advantage) is set,
each sampled mini-batch uses zero-mean, unit-variance advantages before the PPO
loss is evaluated.

## Notes

The actor embeds the configured input shifting, input scaling, and action scaling
directly into the neural-network module so that transferred and checkpointed
controllers operate in the same normalized coordinates used during training.

The trainer updates the actor and critic with separate Adam optimizers and then
broadcasts the updated parameters so all MPI ranks hold the same networks after
each PPO update.

## Example Input File Syntax

!if! function=hasCapability('libtorch')

!syntax parameters /Trainers/LibtorchDRLControlTrainer

!syntax inputs /Trainers/LibtorchDRLControlTrainer

!syntax children /Trainers/LibtorchDRLControlTrainer

!if-end!

!else
!include libtorch/libtorch_warning.md
