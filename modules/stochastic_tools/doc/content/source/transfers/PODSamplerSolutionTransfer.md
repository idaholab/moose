# PODSamplerSolutionTransfer

This object is responsible for transferring solution vectors from [PODFullSolveMultiApp.md]
to [PODReducedBasisTrainer.md] and back.

## Example Syntax

This objects type needs to know about the trainer and the multi-app objects.
This can be achieved by specifying the [!param](/Transfers/PODSamplerSolutionTransfer/trainer_name) and
[!param](/Transfers/PODSamplerSolutionTransfer/to_multi_app) parameters in the
input file.

!listing modules/stochastic_tools/test/tests/surrogates/pod_rb/internal/trainer.i block=Transfers/snapshots

!listing modules/stochastic_tools/test/tests/surrogates/pod_rb/internal/trainer.i block=Transfers/pod_modes

!syntax parameters /Transfers/PODSamplerSolutionTransfer

!syntax inputs /Transfers/PODSamplerSolutionTransfer

!syntax children /Transfers/PODSamplerSolutionTransfer
