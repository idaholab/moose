# TableOutput

The `TableOutput` handles the output of tables to the command line, showing the values of
postprocessors, reporters and scalar variables. It also handles the output of
those same quantities as well as vector postprocessors to CSV files.

## Example output

Table output to the command line is formatted as below for postprocessors and scalar variables
respectively.

```
Postprocessor Values:
+----------------+----------------+----------------+----------------+----------------------+----------------+
| time           | dT             | max_v          | mdot           | total_fission_source | total_power    |
+----------------+----------------+----------------+----------------+----------------------+----------------+
|   0.000000e+00 |   0.000000e+00 |   0.000000e+00 |   0.000000e+00 |         0.000000e+00 |   0.000000e+00 |
|   1.000000e+00 |   1.541793e+01 |   2.101305e+00 |   1.870911e+04 |         6.303327e+01 |   2.999999e+09 |
+----------------+----------------+----------------+----------------+----------------------+----------------+


Scalar Variable Values:
+----------------+----------------+
| time           | lambda         |
+----------------+----------------+
|   0.000000e+00 |   3.501308e-13 |
|   1.000000e+00 |   3.501308e-13 |
+----------------+----------------+
```

And to CSV files:

!listing modules/navier_stokes/test/tests/postprocessors/rayleigh/gold/natural_convection_out.csv
