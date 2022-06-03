# SamplerPostprocessorTransfer

!syntax description /Transfers/SamplerPostprocessorTransfer

## Overview

This object is designed to transfer data from a [Postprocessor](/Postprocessors/index.md)
to a [VectorPostprocessor](/VectorPostprocessors/index.md) on the parent application. This
object +must+ transfer data to a [StochasticResults](/StochasticResults.md)
object.

## Dealing with Failed Solves

When performing stochastic analysis with many perturbations of a sub-application,
it is sometimes the case where the app receives a set of parameters that makes the
solve difficult to converge. With the default configuration of this object, if one
of sub-applications' solve fails, the main application will abort. If it is expected
that some solves might not converge and aborting the main application is not the
desired behavior, the parameter [!param](/MultiApps/SamplerFullSolveMultiApp/ignore_solve_not_converge)
+must+ be set to true in the `MultiApps` block (see [SamplerFullSolveMultiApp.md]
for more details). There are two options for how to transfer results from failed
sub-applications: 1) [!param](/Transfers/SamplerPostprocessorTransfer/keep_solve_fail_value)
is set to false (default) will transfer a NaN to [StochasticResults](/StochasticResults.md)
and 2) [!param](/Transfers/SamplerPostprocessorTransfer/keep_solve_fail_value)
is set to true will transfer whatever the last computed value of the postprocessor was before the solve failed.

## Example Syntax

!listing modules/stochastic_tools/test/tests/transfers/sampler_postprocessor/parent.i block=Transfers

!syntax parameters /Transfers/SamplerPostprocessorTransfer

!syntax inputs /Transfers/SamplerPostprocessorTransfer

!syntax children /Transfers/SamplerPostprocessorTransfer
