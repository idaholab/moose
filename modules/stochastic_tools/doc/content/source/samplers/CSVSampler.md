# CSVSampler

The CSV Sampler object generates samples from a user provided CSV file. The following
assumptions are made about the CSV file:

- All samples are numeric and do not contain strings.
- Each column corresponds to a parameter that is to be sampled. Therefore, multiple
  parameters can be sampled with the same CSV file. 
- All columns have the same number of rows.
- The number of rows in the file is the number of samples for each parameter.

The sampler can either read the entire file or can be used to read specific columns
using the `column_indices` input parameter (see the example syntax below).
The file may or may not have a header.

## Example Input Syntax

!listing modules/stochastic_tools/test/tests/samplers/csv/csv_sampler_indices.i block=Samplers

!syntax parameters /Samplers/CSVSampler

!syntax inputs /Samplers/CSVSampler

!syntax children /Samplers/CSVSampler
