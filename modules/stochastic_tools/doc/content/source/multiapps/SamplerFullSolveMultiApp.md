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

!! min_procs_begin

## Defining Minimum Processors Per App

It is often useful to define the minimum processors to use when running sub-applications.
Typically this is done for large models in batch mode to avoid excessive memory usage.
The [!param](/MultiApps/SamplerFullSolveMultiApp/min_procs_per_app) will utilize this
capability, however it is +required+ that the [!param](/Samplers/MonteCarlo/min_procs_per_row)
parameter in the `Samplers` block be set to the same value. This is to enusre that
the sampler partitioning is equivalent to the multiapp partitioning.

!! min_procs_end

## Skipping Sample Solves

The parameter [!param](/MultiApps/SamplerFullSolveMultiApp/should_run_reporter)
allows for an inputted reporter value to determine whether the sub-app should
be run for a certain sample. And example of using this parameter can be found in
[ConditionalSampleReporter.md].

## Example Syntax

!listing modules/stochastic_tools/test/tests/multiapps/sampler_full_solve_multiapp/parent_full_solve.i block=MultiApps

!syntax parameters /MultiApps/SamplerFullSolveMultiApp

!syntax inputs /MultiApps/SamplerFullSolveMultiApp

!syntax children /MultiApps/SamplerFullSolveMultiApp
