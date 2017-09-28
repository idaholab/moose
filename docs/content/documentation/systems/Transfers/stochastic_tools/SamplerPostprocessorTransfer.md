# SamplerPostprocessorTransfer

This object is designed to transfer data from a [Postprocessor](/Postprocessors/index.md)
to a [VectorPostprocessor](/VectorPostprocessors/index.md) on the master application. This
object **must** transfer data to a [StochasticResults](stochastic_tools/StochasticResults.md)
object.

## Example Syntax
!listing modules/stochastic_tools/test/tests/transfers/sampler_postprocessor/master.i block=Transfers

!syntax parameters /Transfers/SamplerPostprocessorTransfer

!syntax input /Transfers/SamplerPostprocessorTransfer

!syntax children /Transfers/SamplerPostprocessorTransfer
