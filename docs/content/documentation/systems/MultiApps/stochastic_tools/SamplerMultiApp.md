# SamplerMultiApp
The [SamplerMultiApp](#) simply creates a sub application (see [MultiApps]) for each row of
each matrix returned from the [Sampler](stochastic_tools/index.md#samplers) object.

## Example Syntax
!listing modules/stochastic_tools/tests/multiapps/sampler_multiapp/master.i block=MultiApps label=False

!parameters /MultiApps/SamplerMultiApp

!inputfiles /MultiApps/SamplerMultiApp

!childobjects /MultiApps/SamplerMultiApp
