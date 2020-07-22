# ResidualTransfer

This object is responsible for transferring residual vectors from [PODFullSolveMultiApp.md]
to [PODReducedBasisTrainer.md].

## Example Syntax

This objects type needs to know about the trainer and the multi-app objects.
This can be achieved by specifying the `trainer` and `multi_app` parameters in the
input file.

!listing modules/stochastic_tools/test/tests/surrogates/pod_rb/internal/trainer.i block=Transfers/res

!syntax parameters /Transfers/ResidualTransfer

!syntax inputs /Transfers/ResidualTransfer

!syntax children /Transfers/ResidualTransfer
