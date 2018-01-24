# StochasticResults
This object is designed to work with the [SamplerPostprocessorTransfer](/stochastic_tools/SamplerPostprocessorTransfer.md) object for
transferring data from a [Postprocessor](/Postprocessors/index.md) to a
[VectorPostprocessor](/VectorPostprocessors/index.md) on the master application.

## Example Syntax
!listing modules/stochastic_tools/test/tests/transfers/sampler_postprocessor/master.i block=VectorPostprocessors

!syntax parameters /VectorPostprocessors/StochasticResults

!syntax input /VectorPostprocessors/StochasticResults

!syntax children /VectorPostprocessors/StochasticResults
