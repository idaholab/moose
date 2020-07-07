# SamplerSolutionTransfer

This object is responsible for transferring solution vectors from [PODFullSolveMultiApp.md]
to [PODReducedBasisTrainer.md] and back.

## Example Syntax

This objects type needs to know about the trainer and the multi-app objects.
This can be achieved by specifying the `trainer` and `multi_app` parameters in the
input file.

!listing modules/stochastic_tools/test/tests/surrogates/rb_pod/internal/trainer.i block=Transfers/snapshots

!listing modules/stochastic_tools/test/tests/surrogates/rb_pod/internal/trainer.i block=Transfers/pod_modes

!syntax parameters /MultiApps/SamplerSolutionTransfer

!syntax inputs /MultiApps/SamplerSolutionTransfer

!syntax children /MultiApps/SamplerSolutionTransfer
