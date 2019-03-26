# SamplerTransientMultiApp

The [SamplerTransientMultiApp](#) simply creates a transient-type sub application (see [MultiApps]) for each row of
each matrix returned from the [Sampler](stochastic_tools/index.md#samplers) object.

## Example Syntax

!listing modules/stochastic_tools/test/tests/multiapps/sampler_multiapp/master_transient.i block=MultiApps

!syntax parameters /MultiApps/SamplerTransientMultiApp

!syntax inputs /MultiApps/SamplerTransientMultiApp

!syntax children /MultiApps/SamplerTransientMultiApp
