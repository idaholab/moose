# SamplerData
The SamplerData object simply coverts a generated samples from a Sampler into a VectorPostprocessor
vector. One vector is created for each distribution of the sampler and named using the
distribution name.

This object is primarily designed to test the Sampler output.

## Example Syntax
!listing modules/stochastic_tools/tests/samplers/monte_carlo/monte_carlo_uniform.i block=VectorPostprocessors

!parameters /VectorPostprocessors/SamplerData

!inputfiles /VectorPostprocessors/SamplerData

!childobjects /VectorPostprocessors/SamplerData
