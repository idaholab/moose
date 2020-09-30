# SamplerFullSolveMultiApp

!syntax description /MultiApps/SamplerFullSolveMultiApp

## Overview

The [SamplerFullSolveMultiApp](#) simply creates a full-solve type sub application (see [MultiApps])
for each row of each matrix returned from the [Sampler](stochastic_tools/index.md#samplers) object.

This object is capable of running in batch mode by setting the 'mode' parameter. For more
information refer to [batch_mode.md].

## Dealing with Failed Solves

When performing stochastic analysis with many perturbations of a sub-application,
it is sometimes the case where the app receives a set of parameters that makes the
solve difficult to converge. With the default configuration of this object, if one
of sub-applications' solve fails, the main application will abort. This will cause
the main application to stop sampling the sub-application and all the transfers
and processing will fail as well. To prevent this, setting the parameter
[!param](/MultiApps/SamplerFullSolveMultiApp/ignore_solve_not_converge) to true
will allow the main application to continue, despite there being a failed solve.
For options on how failed solve results get transferred to the main application,
see [SamplerPostprocessorTransfer](SamplerPostprocessorTransfer.md). If the
sub-application is a [transient](Transient.md) simulation, the parameter
[!param](/Executioner/Transient/error_on_dtmin) +must+ be set to false in the
`Executioner` block.

## Example Syntax

!listing modules/stochastic_tools/test/tests/multiapps/sampler_full_solve_multiapp/master_full_solve.i block=MultiApps

!syntax parameters /MultiApps/SamplerFullSolveMultiApp

!syntax inputs /MultiApps/SamplerFullSolveMultiApp

!syntax children /MultiApps/SamplerFullSolveMultiApp
