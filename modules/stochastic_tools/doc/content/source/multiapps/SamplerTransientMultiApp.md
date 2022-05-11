# SamplerTransientMultiApp

The [SamplerTransientMultiApp](#) simply creates a transient-type sub application (see [MultiApps]) for each row of
each matrix returned from the [Sampler](stochastic_tools/index.md#samplers) object.

This object is capable of running in batch mode by setting the 'mode' parameter. For more
information refer to [batch_mode.md].

!include SamplerFullSolveMultiApp.md start=min_procs_begin end=min_procs_end

## Example Syntax

!listing modules/stochastic_tools/test/tests/multiapps/sampler_transient_multiapp/parent_transient.i block=MultiApps

!syntax parameters /MultiApps/SamplerTransientMultiApp

!syntax inputs /MultiApps/SamplerTransientMultiApp

!syntax children /MultiApps/SamplerTransientMultiApp
