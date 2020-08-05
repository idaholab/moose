# PODFullSolveMultiApp

This object is responsible for producing snapshots and residuals for [PODReducedBasisTrainer.md].
When the object is executed for the first time, it operates as a [SamplerFullSolveMultiApp.md]
to generate snapshots with different parameter samples. When it is called for the second time,
it generates residuals for given vector tags in each sub-application using the POD modes from
a [PODReducedBasisTrainer.md] object.

## Example Syntax

This MultiApp type needs a [!param](/MultiApps/PODFullSolveMultiApp/trainer_name)
 parameter which allows to access certain
data members of a [PODReducedBasisTrainer.md]. Otherwise, the input syntax is
the same as in case of [SamplerFullSolveMultiApp.md].

!listing modules/stochastic_tools/test/tests/surrogates/pod_rb/internal/trainer.i block=MultiApps

!syntax parameters /MultiApps/PODFullSolveMultiApp

!syntax inputs /MultiApps/PODFullSolveMultiApp

!syntax children /MultiApps/PODFullSolveMultiApp
