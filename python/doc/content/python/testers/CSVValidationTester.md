# CSVValidationTester

The CSVValidationTester can be used for validation purposes.  It compares two `CSV` files: one
produced by a simulation and one containing measured data.  Then, it computes mean and standard
deviation of relative or absolute error between measured and simulated data points.

Users can specify an upper bound for both the mean and std. deviation value via `mean_limit`
and `std_limit` parameters, respectively.

Error type can be either 'relative' or 'absolute' and be set via `err_type` parameter.

Multiple files can be specified in `csvdiff` parameter.

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

## Comparison criteria id=comparison

The tester pools every value of every column except `time`, from each row of the files, into
one set of $N$ pairs of a measured value $v_i$ (from the gold file) and a simulated value $s_i$
(from the test output). With `err_type = relative`, the error of each pair is

!equation id=eq-csvvalidation-rel
e_i = \frac{s_i - v_i}{v_i}

and with `err_type = absolute` it is $e_i = s_i - v_i$. The tester then computes the mean and
the sample standard deviation of the errors,

!equation id=eq-csvvalidation-stats
\bar{e} = \frac{1}{N} \sum_{i=1}^{N} e_i
\qquad
\sigma_e = \sqrt{\frac{1}{N-1} \sum_{i=1}^{N} \left(e_i - \bar{e}\right)^2}

and the test passes if

!equation id=eq-csvvalidation-pass
\bar{e} \le \texttt{mean\_limit} \quad \text{and} \quad \sigma_e \le \texttt{std\_limit}

- The errors are signed, so a negative mean error satisfies any `mean_limit`. Use the absolute
  error type only if the sign of the error is meaningful for the validation.
- With `err_type = relative`, the measured values must be nonzero.
- The tolerances `abs_zero` and `rel_err` of other file testers are not used.
