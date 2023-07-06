# VectorPostprocessorSampler

!syntax description /Samplers/VectorPostprocessorSampler
## Overview

The sampler generates samples from [vector-postprocessor](VectorPostprocessors/index.md) and/or [vector reporters](Reporters/index.md). The user is required to provide the names of these vectors in [!param](/Samplers/VectorPostprocessorSampler/vectors_names), which is in the format `<vpp_name>/<vector_name>` for vector-postprocessors and `<reporter_name>/<value_name>` for reporter values.

## Example Input Syntax

In the example below, a vectorpostprocessor (`[csv]`) are formed using [CSVReader](/CSVReader.md):

!listing modules/stochastic_tools/test/tests/samplers/vectorpostprocessor/vectorpostprocessor.i block=VectorPostprocessors

The VectorPostprocessor Sampler takes vectors `csv/year` and `csv/month` as input to generate samples:

!listing modules/stochastic_tools/test/tests/samplers/vectorpostprocessor/vectorpostprocessor.i block=Samplers

The samples file used in the above examples, `samples.csv`, is listed below.

!listing modules/stochastic_tools/test/tests/samplers/csv/samples.csv

!syntax parameters /Samplers/VectorPostprocessorSampler

!syntax inputs /Samplers/VectorPostprocessorSampler

!syntax children /Samplers/VectorPostprocessorSampler
