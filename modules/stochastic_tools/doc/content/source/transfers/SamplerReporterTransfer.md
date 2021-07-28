# SamplerReporterTransfer

!syntax description /Transfers/SamplerReporterTransfer

## Overview

This object is designed to transfer data from [postprocessors](/Postprocessors/index.md), [vectorpostprocessors](/VectorPostprocessors/index.md), and [reporters](Reporters/index.md) on the sub-application to the main application. This object +must+ transfer data to a [StochasticReporter](/StochasticReporter.md) object.

## How It Works

This transfer works similar to [MultiAppCloneReporterTransfer](MultiAppCloneReporterTransfer.md) whereby creating vector reporter values whose type is based on the type of value being transferred. The name of the reporter values are `<stochastic_reporter name>/<transfer name or prefix>:<sub-app reporter object name>:<sub-app reporter value name>`. In parallel, the vector that the values are being transferred into are distributed by default. For instance, [main_out.json](sampler_reporter/gold/main_out.json) is the output in serial showing all the transferred data; while [main_parallel.json](sampler_reporter/gold/main_parallel.json), [main_parallel.json.1](sampler_reporter/gold/main_parallel.json.1), [main_parallel.json.2](sampler_reporter/gold/main_parallel.json.2), and [main_parallel.json.3](sampler_reporter/gold/main_parallel.json.3) are the combined data from running on 4 processors. However, this split in files can be avoided if [!param](/Reporters/StochasticReporter/parallel_type) in the reporter object is set to `ROOT`. This will gather all the data to the root processor, beware using this option can cause memory issues with very very large stochastic runs.

## Dealing with Failed Solves

When performing stochastic analysis with many perturbations of a sub-application, it is sometimes the case where the app receives a set of parameters that makes the solve difficult to converge. With the default configuration of this object, if one of sub-applications' solve fails, the main application will abort. If it is expected that some solves might not converge and aborting the main application is not the desired behavior, the parameter [!param](/MultiApps/SamplerFullSolveMultiApp/ignore_solve_not_converge) +must+ be set to true in the `MultiApps` block (see [SamplerFullSolveMultiApp.md] for more details). With this parameter set to true in the multiapp, this object will transfer whatever the last compute values are. To keep track of whether the solve converged or not, the reporter value `<stochastic_reporter name>/multiapp_converged` is created. See [main_out.json](sampler_reporter/gold/main_out.json) as an example.

## Example Syntax

!listing modules/stochastic_tools/test/tests/transfers/sampler_reporter/main.i block=MultiApps Transfers Reporters

!syntax parameters /Transfers/SamplerReporterTransfer

!syntax inputs /Transfers/SamplerReporterTransfer

!syntax children /Transfers/SamplerReporterTransfer
