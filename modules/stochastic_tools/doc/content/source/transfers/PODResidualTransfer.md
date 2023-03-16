# PODResidualTransfer

This object is responsible for transferring residual vectors from [PODFullSolveMultiApp.md]
to [PODReducedBasisTrainer.md].

## Example Syntax

This objects type needs to know about the trainer and the multi-app objects.
This can be achieved by specifying the [!param](/Transfers/PODResidualTransfer/trainer_name) and
[!param](/Transfers/PODResidualTransfer/to_multi_app) parameters in the
input file.

!listing modules/stochastic_tools/test/tests/surrogates/pod_rb/internal/trainer.i block=Transfers/res

!syntax parameters /Transfers/PODResidualTransfer

!syntax inputs /Transfers/PODResidualTransfer

!syntax children /Transfers/PODResidualTransfer
