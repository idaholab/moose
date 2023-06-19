# VectorPostprocessorSampler

The VectorPostprocessor Sampler object generates samples from [VectorPostprocessors](syntax/Postprocessors/index.md). The user is required to provide a vector from [VectorPostprocessors](syntax/Postprocessors/index.md) in [!param](/Samplers/VectorPostprocessorSampler/vectors_names). The data type of the input vector need to be real.

## Example Input Syntax

In the example below, two input vectorpostprocessors (`[VPP1]` and `[VPP2]`) are formed using [CSVReader](/CSVReader.md):

!listing modules/stochastic_tools/test/tests/samplers/vectorpostprocessor/vectorpostprocessor.i block=VectorPostprocessors

The VectorPostprocessor Sampler takes vectors from `[VPP1]` and `[VPP2]` as input to generate samples:

!listing modules/stochastic_tools/test/tests/samplers/vectorpostprocessor/vectorpostprocessor.i block=Samplers

The samples file used in the above examples, `samples.csv`, is listed below.

!listing modules/stochastic_tools/test/tests/samplers/csv/samples.csv

!syntax parameters /Samplers/VectorPostprocessorSampler

!syntax inputs /Samplers/VectorPostprocessorSampler

!syntax children /Samplers/VectorPostprocessorSampler
