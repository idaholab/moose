<!-- MOOSE Documentation Stub: Remove this when content is added. -->

# PerformanceData

The PerformanceData Postprocessor is designed to retrieve information from
the performance log as a Postprocessor value. The performance log is normally
printed to the screen in a table format, which is nontrival to parse.

```
 -------------------------------------------------------------------------------------------------------------------------
| Moose Test Performance: Alive time=0.356467, Active time=0.341974                                                       |
 -------------------------------------------------------------------------------------------------------------------------
| Event                                      nCalls     Total Time  Avg Time    Total Time  Avg Time    % of Active Time  |
|                                                       w/o Sub     w/o Sub     With Sub    With Sub    w/o S    With S   |
|-------------------------------------------------------------------------------------------------------------------------|
|                                                                                                                         |
|                                                                                                                         |
| Application                                                                                                             |
|   Full Runtime                             1          0.0077      0.007692    0.3420      0.342041    2.25     100.02   |
|                                                                                                                         |
| Execution                                                                                                               |
|   computeKernels()                         279        0.1787      0.000641    0.1787      0.000641    52.27    52.27    |
|   computeNodalBCs()                        279        0.0066      0.000023    0.0066      0.000023    1.92     1.92     |
|   compute_jacobian()                       40         0.0466      0.001164    0.0466      0.001164    13.61    13.61    |
|   compute_residual()                       279        0.0151      0.000054    0.2005      0.000719    4.42     58.62    |
|   solve()                                  20         0.0399      0.001993    0.2869      0.014345    11.65    83.90    |
|                                                                                                                         |
| Output                                                                                                                  |
|   Exodus::output()                         21         0.0142      0.000676    0.0142      0.000676    4.15     4.15     |
|                                                                                                                         |
| Setup                                                                                                                   |
|   Application Setup                        1          0.0291      0.029063    0.0320      0.032037    8.50     9.37     |
|   FEProblemBase::init::meshChanged()       1          0.0002      0.000208    0.0002      0.000208    0.06     0.06     |
|   Initial updateActiveSemiLocalNodeRange() 1          0.0000      0.000031    0.0000      0.000031    0.01     0.01     |
|   Initial updateGeomSearch()               2          0.0000      0.000001    0.0000      0.000001    0.00     0.00     |
|   NonlinearSystem::update()                1          0.0001      0.000072    0.0001      0.000072    0.02     0.02     |
|   eq.init()                                1          0.0027      0.002694    0.0027      0.002694    0.79     0.79     |
|   execMultiApps()                          1          0.0000      0.000008    0.0000      0.000008    0.00     0.00     |
|   execTransfers()                          1          0.0000      0.000001    0.0000      0.000001    0.00     0.00     |
|   initial adaptivity                       1          0.0000      0.000003    0.0000      0.000003    0.00     0.00     |
|   initialSetup()                           1          0.0008      0.000773    0.0012      0.001215    0.23     0.36     |
|   reinit() after updateGeomSearch()        1          0.0000      0.000003    0.0000      0.000003    0.00     0.00     |
|                                                                                                                         |
| Utility                                                                                                                 |
|   projectSolution()                        1          0.0004      0.000394    0.0004      0.000394    0.12     0.12     |
 -------------------------------------------------------------------------------------------------------------------------
| Totals:                                    932        0.3420                                          100.00            |
 -------------------------------------------------------------------------------------------------------------------------
```

# Usage

To print an individaul timing, refer to it by specifying the desired row and column. For instance, if we wanted
to print the "total time with sub" for "compute_jacobian()", we'd use this syntax:

!listing postprocessors/print_perf_data/print_perf_data.i block=Postprocessors/jac_total_time_with_sub

For a complete list of column names, refer to the listing here:

!listing postprocessors/PerformanceData.C re=MooseEnum\scolumn_options.*?;

!alert note
The `Outputs/print_perf_log` option does +not+ have to be set to `true` in order
for this Postprocessor to work. Logging is enabled in the background for the
purpose of supplying data to this postprocessor, but not printed to the screen.

## Syntax and Description.

!syntax description /Postprocessors/PerformanceData

!syntax parameters /Postprocessors/PerformanceData

!syntax inputs /Postprocessors/PerformanceData

!syntax children /Postprocessors/PerformanceData

!bibtex bibliography
