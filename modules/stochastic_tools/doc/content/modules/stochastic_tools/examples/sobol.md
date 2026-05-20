# SOBOL Sensitivity Analysis

The following example converts [parameter_study.md] to perform a global variance-based
sensitivity analysis following the method of [!cite](saltelli2002making).

## Input File

To perform a the sensitivity analysis only two minor changes to the input file from
[parameter_study.md] need to occur: the sampler needs be changed and another statistics
calculation object needs to be added, the remainder of the file remains the same. The exception
being that the sampler name in a few of the objects was updated to use the "sobol" Sampler
name, as defined in the following sub-section.

### Sobol Sampler

The Sobol sampling scheme consists of using a sample and re-sample matrix to create a series of
matrices that can be used to compute first-order, second-order, and total-effect sensitivity indices.
The first-order indices are the portion of the variance in the quantity of interest (e.g., $T_{avg}$)
due to the variance of the uncertain parameters (e.g., $\gamma$). The second-order indices are the
portion of the variance in the quantity of interest due to the variance two uncertain parameters
interacting (e.g., $\gamma$ as $s$).  Finally, the total-effect indices are the portion of the
variance of the quantity of interest due to the variance of an uncertain parameter and all
interactions with the other parameters. For complete details of the method please refer to
[!cite](saltelli2002making).

The [SobolSampler.md] object requires two input samplers to form the sample and re-sample matrix.
Thus, the `Samplers` block contains three sample objects. The first two are used by the third,
the "sobol" sampler, which is the Sampler used by the other objects in the simulation.

!listing examples/sobol/main.i block=Samplers

The sobol method implemented here requires $n(2k+2)$ model evaluations, where $k$ is the number
of uncertain parameters (4) and $n$ is the number of replicates (10,000). Therefore, for this example
100,000 model evaluations were performed to compute the indices.

### Sobol Statistics

The values of the first-order, second-order, and total-effect are computed by the
[SobolReporter.md] object. This object is a Reporter, as such it is added to the
`Reporters` block. The output of the object includes all the indices for each of
the vectors supplied.

!listing examples/sobol/main.i block=Reporters

## Results

If [JSONOutput.md] output is enabled, the [SobolReporter.md] object will write a file that contains
columns of data. Each column comprises of the computed indices for the quantities of
interest. For example, [sobol_out] is and example output from the [SobolReporter.md] object
for this example problem.

!listing examples/sobol/gold/main_out.json language=json
         caption=Computed Sobol indices for the example heat conduction problem.
         id=sobol_out

For each set of indices (`FIRST_ORDER`, `SECOND_ORDER`, and `TOTAL`) contains a pair entries:
the first is the values computed and the second corresponds to the 5% and 95% confidence intervals.
This problem examines four uncertain parameters, so each element of the set of indices corresponds
to the parameter indicated in the input file ($\gamma$, $q_0$, $T_0$, and $s$). See [SobolReporter.md]
for further information regarding the output.

For the problem at hand, the first-order and second-order indices for the two quantities of interest
are presented in [S_T_avg] and [S_q_left]. The diagonal entries are the first-order indices and
the off-diagonal terms are the second-order indices. For example for $T_{avg}$ the first order-index
for $\gamma$ is $S_1 = 0.763$ and the second-order index $S_{1,2} = 0.014$ for $\gamma$ interacting
with $q_0$. The negative values are essentially zero, if more replicates were executed these
numbers would become closer to zero.

```
python ../../python/moose_stochastic_tools/visualize_sobol.py main_out.json  --markdown-table \
--values results_results:T_avg:value --stat second_order \
--param-names '$\gamma$' '$q_0$' '$T_0$' '$s$' \
--number-format .3g
```

!table id=S_T_avg caption=First-order and second-order Sobol indices for $T_{avg}$.
| $S_{i,j}$ (5.0%, 95.0%) CI   | $\gamma$               | $q_0$                         | $T_0$                      | $s$                  |
|:-----------------------------|:-----------------------|:------------------------------|:---------------------------|:---------------------|
| $\gamma$                     | 0.687 (0.625, 0.765)   | -                             | -                          | -                    |
| $q_0$                        | 0.0138 (-0.11, 0.138)  | 0.00735 (0.00595, 0.00887)    | -                          | -                    |
| $T_0$                        | 0.0212 (-0.115, 0.159) | -0.000649 (-0.00788, 0.00631) | 0.0898 (0.0803, 0.101)     | -                    |
| $s$                          | 0.0111 (-0.147, 0.171) | -0.00372 (-0.0183, 0.0107)    | -0.00414 (-0.0285, 0.0203) | 0.206 (0.186, 0.231) |

```
python ../../python/moose_stochastic_tools/visualize_sobol.py main_out.json  --markdown-table \
--values results_results:q_left:value --stat second_order \
--param-names '$\gamma$' '$q_0$' '$T_0$' '$s$' \
--number-format .3g
```

!table id=S_q_left caption=First-order and second-order Sobol indices for $q_{left}$.
| $S_{i,j}$ (5.0%, 95.0%) CI   | $\gamma$                | $q_0$                         | $T_0$                         | $s$                     |
|:-----------------------------|:------------------------|:------------------------------|:------------------------------|:------------------------|
| $\gamma$                     | 0.822 (0.771, 0.88)     | -                             | -                             | -                       |
| $q_0$                        | 0.00132 (-0.1, 0.104)   | 0.0286 (0.0262, 0.0312)       | -                             | -                       |
| $T_0$                        | 0.00552 (-0.108, 0.12)  | -0.00308 (-0.00887, 0.00257)  | 0.134 (0.125, 0.145)          | -                       |
| $s$                          | 7.92e-05 (-0.0992, 0.1) | -0.00201 (-0.00447, 0.000333) | -0.000977 (-0.00868, 0.00665) | 0.0049 (0.0038, 0.0061) |

The data in these two tables clearly indicates that a majority of the variance of both quantities of
interest are due to the variance of $\gamma$, $s$ also affects $T_{avg}$ and $T_0$ affects $q_{left}$.
Additionally, a small contribution of the variance is from a second-order interaction, $S_{1,2}$, between $\gamma$ and
$T_0$. The importance of $\gamma$, $T_0$, and $s$ is further evident by the total-effect indices, as shown in
[total-effect].

```
python ../../python/moose_stochastic_tools/visualize_sobol.py main_out.json --markdown-table --stat total \
--names '{"results_results:T_avg:value":"$T_{avg}$", "results_results:q_left:value":"$q_{left}$"}' \
--param-names '$\gamma$' '$q_0$' '$T_0$' '$s$' \
--number-format .3g
```

!table id=total-effect caption=Total-effect Sobol indices for $T_{avg}$ and $q_{left}$.
| $S_T$ (5.0%, 95.0%) CI   | $\gamma$             | $q_0$                    | $T_0$                  | $s$                      |
|:-------------------------|:---------------------|:-------------------------|:-----------------------|:-------------------------|
| $T_{avg}$                | 0.699 (0.665, 0.728) | 0.0131 (-0.0955, 0.102)  | 0.108 (0.00979, 0.189) | 0.208 (0.121, 0.28)      |
| $q_{left}$               | 0.836 (0.823, 0.846) | 0.0406 (-0.0249, 0.0979) | 0.158 (0.0999, 0.208)  | 0.0149 (-0.0517, 0.0736) |

To help visualize the sensitivities, `visualize_sobol.py` can also represent it as a bar pot or heat map:

```
python ../../python/moose_stochastic_tools/visualize_sobol.py main_out.json --bar-plot --log-scale --stat total \
--names '{"results_results:T_avg:value":"$T_{avg}$", "results_results:q_left:value":"$q_{left}$"}' \
--param-names '$\gamma$' '$q_0$' '$T_0$' '$s$'
```

!media stochastic_tools/sobol/sobol_bar.png
       alt=Bar graph showing the sensitivity of the heat flux and the average temperature to different variables.

```
python ../../python/moose_stochastic_tools/visualize_sobol.py main_out.json --heatmap --log-scale --stat second_order \
--names '{"results_results:T_avg:value":"$T_{avg}$", "results_results:q_left:value":"$q_{left}$"}' \
--param-names '$\gamma$' '$q_0$' '$T_0$' '$s$'
```

!media stochastic_tools/sobol/sobol_heatmap.png
       alt=Heat map showing the sensitivity of the heat flux and the average temperature to different variables.
