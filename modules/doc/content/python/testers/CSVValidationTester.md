# CSVValidationTester

The CSVValidationTester can be used for validation purposes.  It compares two `CSV` files: one
produced by a simulation and one containing measured data.  Then, it computes mean and standard
deviation of relative or absolute error between measured and simulated data points.

Users can specify an upper bound for both the mean and std. deviation value via `mean_limit`
and `std_limit` parameters, respectively.

Error type can be either 'relative' or 'absolute' and be set via `err_type` parameter.

Multiple files can be specified in `cvsdiff` parameter.

For inspecting the values, users can add `-v` flag when running the `run_tests` scripts and they
can check the results that will come out in a table like this:

```
file                                     | computed             | requested
--------------------------------------------------------------------------------------
test_15.csv                              | 0.91 +/- 1.          | 1.00 +/- 2.00
```

Here `file` is the file name from `csvdiff` parameter, `computed` is the mean and std. deviation
of the simulation values and `requested` is what was prescribed via `mean_limit` and `std_limit`

Note that currently only steady-state comparison is supported.
