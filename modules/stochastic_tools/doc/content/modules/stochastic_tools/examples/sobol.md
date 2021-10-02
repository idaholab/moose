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
are presented in [S_T_avg] and [S_q_left]. The diagonal entries are the first-order incides and
the off-diagonal terms are the second-order indices. For example for $T_{avg}$ the first order-index
for $\gamma$ is $S_1 = 0.763$ and the second-order index $S_{1,2} = 0.014$ for $\gamma$ interacting
with $q_0$. The negative values are essentially zero, if more replicates were executed these
numbers would become closer to zero.

```
python ../../python/visualize_sobol.py main_out.json  --markdown-table \
--values results_results:T_avg:value --stat second_order \
--param-names '$\gamma$' '$q_0$' '$T_0$' '$s$' \
--number-format .3g
```

!table id=S_T_avg caption=First-order and second-order Sobol indices for $T_{avg}$.
| $S_{i,j}$ (5.0%, 95.0%) CI   | $\gamma$                | $q_0$                      | $T_0$                     | $s$                  |
|:-----------------------------|:------------------------|:---------------------------|:--------------------------|:---------------------|
| $\gamma$                     | 0.69 (0.626, 0.77)      | -                          | -                         | -                    |
| $q_0$                        | 0.00108 (-0.122, 0.122) | 0.00737 (0.00598, 0.00892) | -                         | -                    |
| $T_0$                        | 0.0114 (-0.126, 0.148)  | 0.00412 (-0.003, 0.0111)   | 0.0902 (0.0807, 0.102)    | -                    |
| $s$                          | 0.00538 (-0.153, 0.16)  | 0.0039 (-0.0101, 0.018)    | 0.00353 (-0.0207, 0.0283) | 0.201 (0.181, 0.225) |

```
python ../../python/visualize_sobol.py main_out.json  --markdown-table \
--values results_results:q_left:value --stat second_order \
--param-names '$\gamma$' '$q_0$' '$T_0$' '$s$' \
--number-format .3g
```

!table id=S_q_left caption=First-order and second-order Sobol indices for $q_{left}$.
| $S_{i,j}$ (5.0%, 95.0%) CI   | $\gamma$                | $q_0$                         | $T_0$                         | $s$                        |
|:-----------------------------|:------------------------|:------------------------------|:------------------------------|:---------------------------|
| $\gamma$                     | 0.815 (0.765, 0.874)    | -                             | -                             | -                          |
| $q_0$                        | 0.00787 (-0.094, 0.108) | 0.0267 (0.0245, 0.0292)       | -                             | -                          |
| $T_0$                        | 0.0188 (-0.0959, 0.132) | -0.000773 (-0.00685, 0.00532) | 0.135 (0.126, 0.145)          | -                          |
| $s$                          | 0.011 (-0.0887, 0.109)  | 0.000268 (-0.0021, 0.00265)   | -0.000539 (-0.00844, 0.00736) | 0.00423 (0.00312, 0.00538) |

The data in these two tables clearly indicates that a majority of the variance of both quantities of
interest are due to the variance of $\gamma$, $s$ also affects $T_{avg}$ and $T_0$ affects $q_{left}$.
Additionally, a small contribution of the variance is from a second-order interaction, $S_{1,2}$, between $\gamma$ and
$T_0$. The importance of $\gamma$, $T_0$, and $s$ is further evident by the total-effect indices, as shown in
[total-effect].

```
python ../../python/visualize_sobol.py main_out.json --markdown-table --stat total \
--names '{"results_results:T_avg:value":"$T_{avg}$", "results_results:q_left:value":"$q_{left}$"}' \
--param-names '$\gamma$' '$q_0$' '$T_0$' '$s$' \
--number-format .3g
```

!table id=total-effect caption=Total-effect Sobol indices for $T_{avg}$ and $q_{left}$.
| $S_T$ (5.0%, 95.0%) CI   | $\gamma$             | $q_0$                    | $T_0$                 | $s$                      |
|:-------------------------|:---------------------|:-------------------------|:----------------------|:-------------------------|
| $T_{avg}$                | 0.707 (0.673, 0.736) | 0.0217 (-0.0861, 0.111)  | 0.115 (0.016, 0.196)  | 0.208 (0.12, 0.28)       |
| $q_{left}$               | 0.837 (0.825, 0.848) | 0.0415 (-0.0242, 0.0999) | 0.157 (0.0988, 0.209) | 0.0137 (-0.0537, 0.0743) |

To help visualize the sensitivities, `visualize_sobol.py` can also represent it as a bar pot or heat map:

```
python ../../python/visualize_sobol.py main_out.json --bar-plot --log-scale --stat total \
--names '{"results_results:T_avg:value":"$T_{avg}$", "results_results:q_left:value":"$q_{left}$"}' \
--param-names '$\gamma$' '$q_0$' '$T_0$' '$s$'
```

!media stochastic_tools/sobol/sobol_bar.png

```
python ../../python/visualize_sobol.py main_out.json --heatmap --log-scale --stat second_order \
--names '{"results_results:T_avg:value":"$T_{avg}$", "results_results:q_left:value":"$q_{left}$"}' \
--param-names '$\gamma$' '$q_0$' '$T_0$' '$s$'
```

!media stochastic_tools/sobol/sobol_heatmap.png
