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

!listing examples/sobol/master.i block=Samplers

The sobol method implemented here requires $n(2k+2)$ model evaluations, where $k$ is the number
of uncertain parameters (4) and $n$ is the number of replicates (10,000). Therefore, for this example
100,000 model evaluations were performed to compute the indices.

### Sobol Statistics

The values of the first-order, second-order, and total-effect are computed by the
[SobolStatistics.md] object. This object is a VectorPostprocessor, as such it is added to the
`VectorPostprocessors` block. The output of the object includes all the indices for each of
the vectors supplied.

## Results

If CSV output is enabled, the [SobolStatistics.md] object will write a file that contains
columns of data. Each column comprises of the computed indices for the quantities of
interest. For example, [sobol_out] is the complete output from the [SobolStatistics.md] object
for this example problem.

!listing caption=Computed Sobol indices for the example heat conduction problem. id=sobol_out
results_results:T_avg,results_results:q_left
0.78281576441957,0.77757846638372
0.20244571666716,0.20835461645184
0.0028971365514415,0.001249518487784
0.00057637952893031,-7.7803918689059e-06
0.7962125505073,0.79310223868474
0.22205277595128,0.23142827171255
0.0078278880010163,0.0074376466338467
0.0057602099050202,0.0064422786852175
0.0013657906960507,0.0023676473484507
-0.026286609856123,-0.028546434327499
-0.027093927368103,-0.0288934212785
-0.0038658094816267,-0.0039973359755034
-0.0034417727808675,-0.0036586486293979
-2.439285675197e-05,2.2538777552327e-05

This problem examines four uncertain parameters, thus the first four rows contain the first-order
indices of each of the uncertain parameters as ordered in the input file ($\gamma$, $q_0$, $T_0$, and
$s$). The next four rows contain the total-effect indices. The final rows contain the second-order
indices, see [SobolStatistics.md] for further information regarding the output.

For the problem at hand, the first-order and second-order indices for the two quantities of interest
are presented in [S_T_avg] and [S_q_left]. The diagonal entries are the first-order incides and
the off-diagonal terms are the second-order indices. For example for $T_{avg}$ the first order-index
for $\gamma$ is $S_1 = 0.763$ and the second-order index $S_{1,2} = 0.014$ for $\gamma$ interacting
with $q_0$. The negative values are essentially zero, if more replicates were executed these
numbers would become closer to zero.

!table id=S_T_avg caption=First-order and second-order Sobol indices for $T_{avg}$.
| $S_{i,j}$ | 1 ($\gamma$) | 2 ($q_0$) | 3 ($T_0$) | 4 ($s$) |
| -         | -            | -         | -         | -       |
| 1         | 0.783        | -         | -         | -       |
| 2         | 0.014        | 0.202     | -         | -       |
| 3         | -0.026       | -0.004    | 0.003     | -       |
| 4         | -0.027       | -0.003    | 0         | 0.001   |

!table id=S_q_left caption=First-order and second-order Sobol indices for $q_{left}$.
| $S_{i,j}$ | 1 ($\gamma$) | 2 ($q_0$) | 3 ($T_0$) | 4 ($s$) |
| -         | -            | -         | -         | -       |
| 1         | 0.778        | -         | -         | -       |
| 2         | 0.002        | 0.215     | -         | -       |
| 3         | -0.029       | -0.004    | 0.001     | -       |
| 4         | -0.029       | -0.004    | 0         | 0       |

The data in these two tables clearly indicates that a majority of the variance of both quantities of
interest are due to the variance of $\gamma$ ($S_1$) and $q_0$ ($S_2$). Additionally, a small
contribution of the variance is from a second-order interaction, $S_{1,2}$, between $\gamma$ and
$q_0$. The importance of $\gamma$ and $q_0$ if further evident by the total-effect indices, as shown in
[total-effect].

!table id=total-effect caption=Total-effect Sobol indices for $T_{avg}$ and $q_{left}$.
| $S_T$      | 1 ($\gamma$) | 2 ($q_0$) | 3 ($T_0$) | 4 ($s$) |
| -          | -            | -         | -         | -       |
| $T_{avg}$  | 0.796        | 0.222     | 0.008     | 0.006   |
| $q_{left}$ | 0.793        | 0.231     | 0.007     | 0.006   |
