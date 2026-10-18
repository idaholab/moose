# SamplerDRLControlTransfer

!if! function=hasCapability('libtorch')

!syntax description /Transfers/SamplerDRLControlTransfer

## Overview

`SamplerDRLControlTransfer` copies the current actor network of a
[LibtorchDRLControlTrainer](LibtorchDRLControlTrainer.md) on the main application
into a [LibtorchDRLControl](LibtorchDRLControl.md) object on every sub-application
of a sampler-driven MultiApp, such as
[SamplerFullSolveMultiApp](SamplerFullSolveMultiApp.md). Each sub-application then
runs one rollout of the latest policy, and the resulting trajectories are sent back
to the trainer for the next policy update.
[!param](/Transfers/SamplerDRLControlTransfer/trainer_name) names the trainer on the
main application, and [!param](/Transfers/SamplerDRLControlTransfer/control_name)
names the control in the sub-application input. After the network is copied, the
transfer executes the control once, so that each rollout starts from an action of
the current policy.

When the MultiApp runs in `batch-reset` or `batch-restore` mode, the transfer also
reseeds the stochastic action sampling of the control for every sampler row. The
seed is built from the trainer's
[!param](/Trainers/LibtorchDRLControlTrainer/seed), the sampler row index, and the
main-application time step. Every rollout of every training iteration thus draws
its own action sequence, and that sequence does not depend on how the sampler rows
are distributed among processors.

The transfer checks that the control executes the policy the trainer learns, and
reports an error unless all of the following hold:

- [!param](/Trainers/LibtorchDRLControlTrainer/timestep_window) equals
  [!param](/Controls/LibtorchDRLControl/num_steps_in_period), so that each training
  transition corresponds to exactly one policy evaluation;
- [!param](/Trainers/LibtorchDRLControlTrainer/input_timesteps) equals
  [!param](/Controls/LibtorchDRLControl/input_timesteps), so that the control stacks
  the same observation history the actor was trained on;
- the number of reporters in
  [!param](/Trainers/LibtorchDRLControlTrainer/observation) equals the number of
  postprocessors in [!param](/Controls/LibtorchDRLControl/observations); and
- the number of reporters in [!param](/Trainers/LibtorchDRLControlTrainer/control)
  equals the number of controllable parameters in
  [!param](/Controls/LibtorchDRLControl/parameters).

To transfer a trained network into a deterministic
[LibtorchNeuralNetControl](LibtorchNeuralNetControl.md) instead, use
[LibtorchNeuralNetControlTransfer](LibtorchNeuralNetControlTransfer.md).

## Example Input Syntax

In this example, the transfer copies the actor of the `nn_trainer` trainer into the
`src_control` control of each `runner` sub-application before the sub-applications
run. The `dummy` sampler has four rows, so every training iteration collects four
rollouts in batch mode.

!listing modules/stochastic_tools/test/tests/transfers/sampler_drl_control_transfer/trainer.i block=Transfers/nn_transfer

The control on the sub-application evaluates the policy every two time steps on a
two-step observation history, matching the trainer's `timestep_window = 2` and
`input_timesteps = 2`.

!listing modules/stochastic_tools/test/tests/transfers/sampler_drl_control_transfer/sub.i block=Controls

!syntax parameters /Transfers/SamplerDRLControlTransfer

!syntax inputs /Transfers/SamplerDRLControlTransfer

!syntax children /Transfers/SamplerDRLControlTransfer

!if-end!

!else
!include libtorch/libtorch_warning.md
