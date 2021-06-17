# CSVSampler

The CSV Sampler object generates samples from a user provided CSV file. The following
assumptions are made about the CSV file:

- All samples are numeric and do not contain strings.
- Each column corresponds to a parameter that is to be sampled. Therefore, multiple
  parameters can be sampled with the same CSV file.
- All columns have the same number of rows.
- The number of rows in the file is the number of samples for each parameter.

The sampler can either read the entire file or can be used to read specific columns
using the `column_indices` or the `column_names` input parameters (see the example
syntax below). `column_indices` refer to the indices of the columns and
`column_names` refer to the header names of the columns in the CSV file.

## Example Input Syntax

In the example below, the sampler reads a file, `samples.csv`, which contains
samples for various parameters. The input below reads columns 0, 1, and 3 from
the samples file.

!listing modules/stochastic_tools/test/tests/samplers/csv/csv_sampler_indices.i block=Samplers

In another example below, the sampler reads the columns with the column names,
`a`, `b`, and `d`.

!listing modules/stochastic_tools/test/tests/samplers/csv/csv_sampler_names.i block=Samplers

The samples file used in the above examples, `samples.csv`, is listed below.

!listing modules/stochastic_tools/test/tests/samplers/csv/samples.csv

!syntax parameters /Samplers/CSVSampler

!syntax inputs /Samplers/CSVSampler

!syntax children /Samplers/CSVSampler
