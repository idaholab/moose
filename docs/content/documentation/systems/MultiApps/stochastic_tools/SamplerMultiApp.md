# SamplerMultiApp
The [SamplerMultiApp](#) simply creates a sub application (see [MultiApps]) for each row of
each matrix returned from the [Sampler](stochastic_tools/index.md#samplers) object.

## Example Syntax
!listing modules/stochastic_tools/test/tests/multiapps/sampler_multiapp/master.i block=MultiApps label=False

!syntax parameters /MultiApps/SamplerMultiApp

!syntax input /MultiApps/SamplerMultiApp

!syntax children /MultiApps/SamplerMultiApp
