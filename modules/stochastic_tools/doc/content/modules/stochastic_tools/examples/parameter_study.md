# Parameter Study

A parameter study is defined here as performing a simulation with varying uncertain
parameters and computing quantities of interest for each set of uncertain parameters.
This example is a more detailed example of what is presented in [examples/monte_carlo.md], which
is also a parameter study.

## Problem Statement

To demonstrate how to perform a parameter study the transient heat equation will be used.

!equation id=diffusion-strong
\frac{\partial T}{\partial t} - \nabla\cdot\gamma\nabla T + s = 0,

where $T$ is temperature, $t$ is time, $\gamma$ is diffusivity, and $s$ is a heat source term.
This problem shall be solved on a 2D unit square domain. The left side is subjected
to a Dirichlet condition $T=T_0$. The right side of the domain is subject to a Neumann condition
$-\gamma\nabla T \cdot \hat{n} = q_0$, where $\hat{n}$ is the outward facing normal vector from the
boundary.

The boundary conditions $T_0$ and $q_0$ as well as the diffusivity ($\gamma$) and the source
term $s$ are assumed to be known. However, these parameters have some associated uncertainty.
The effect of the uncertainty on the average temperature and heat flux on the left boundary after
one second of simulation time is desired, defined as $T_{avg} = \bar{T}(t=1)$ and
$q_{left} = q(t=1,x=0)$.

## Heat Equation Problem

The first step in performing a parameter study is to create an input file that solves your problem
without uncertain parameters. For the example problem this will be done assuming $\gamma=1$,
$s=100$, $T_0=300$, and $q_0=-100$.

The complete input file for this problem is provided in [diffusion]. The only item
that is atypical from a MOOSE simulation input file is the existence of the `Controls` block, which
here simply creates a [SamplerReceiver.md] object. This block is required for the parameter study,
but shall be discussed in the [parameter_study.md#transfers] section.

!listing parameter_study/diffusion.i id=diffusion
         caption=Complete input file for example heat equation problem to be used in parameter study.

Executing this file using the StochasticToolsApp can be done as follows

```language=sh
cd moose/modules/stochastic_tools/examples/parameter_study
../../stochastic_tools-opt -i diffusion.i
```

The simulation will perform four timesteps and should result in $T_{avg}=286$ and $q_{left}=1.73$.


## Main Input

To perform a parameter study an input file that will drive the stochastic simulations is required.
This file will be referred to as the main input file and is responsible for defining the
uncertain parameters, running the stochastic simulations, and collecting the stochastic results. The
following sub-sections will step through each portion of the main file to explain the purpose. The
complete analysis is discussed in [parameter_study.md#results].

The specific parameter study performed in this tutorial can also done using the [ParameterStudy/index.md] syntax, shown in the code block below. However, it is very useful to learn each aspect of doing a stochastic simulation in the stochastic tools module, which is described in the following sub-sections.

!listing examples/parameter_study/main_parameter_study.i caption=Parameter study using `ParameterStudy` input block

### StochasticTools Block

The [StochasticTools](syntax/StochasticTools/index.md) block sets up the main file to be a
driver for stochastic simulations but itself is not performing any sort of simulation. Setting up
a main application without a simulation itself is the default behavior for this block, as such it
is empty in this example.

!listing examples/parameter_study/main.i block=StochasticTools

### Distributions and Samplers Blocks

The [Distributions](syntax/Distributions/index.md) block defines the statistical distribution
for each of the uncertain parameters ($T_0$, $q_0$, $\gamma$, and $s$) to be defined. For this
example, the diffusivity ($\gamma$) is defined by a uniform distribution, the flux ($q_0$) by
a three-parameter Weibull, and $T_0$ and $s$ a normal distribution.

!listing examples/parameter_study/main.i block=Distributions

The [Samplers](syntax/Samplers/index.md) block defines how the distributions shall be sampled for the
parameter study. In this case a Latin hypercube sampling strategy is employed. A total of 5000 samples
are being used.

!listing examples/parameter_study/main.i block=Samplers

For this problem the Sampler object is setup to run in "batch-restore" mode, which is a mode of
operation for memory efficient creation of sub-applications. Please see
[stochastic_tools/batch_mode.md] for more information.

The input distributions for each of these four terms are included in [sampler_gamma]--[sampler_s]
for 5000 samples.

!plot histogram filename=stochastic_tools/parameter_study/main_out_samples_0002.csv
                vectors=hypercube_0
                xlabel=Diffusivity
                id=sampler_gamma
                legend=False
                caption=Input distribution of diffusivity ($\gamma$).

!plot histogram filename=stochastic_tools/parameter_study/main_out_samples_0002.csv
                vectors=hypercube_1
                xlabel=Flux
                id=sampler_q_0
                legend=False
                caption=Input distribution of flux boundary condition ($q_0$).

!plot histogram filename=stochastic_tools/parameter_study/main_out_samples_0002.csv
                vectors=hypercube_2
                xlabel=Temperature
                id=sampler_T_0
                legend=False
                caption=Input distribution of temperature boundary condition ($T_0$).

!plot histogram filename=stochastic_tools/parameter_study/main_out_samples_0002.csv
                vectors=hypercube_3
                xlabel=Source
                id=sampler_s
                legend=False
                caption=Input distribution of source term ($s$).

### Transfers Block id=transfers

The [Transfers](syntax/Transfers/index.md) block serves two purposes. First, the "parameters"
sub-block instantiates a [SamplerParameterTransfer.md] object that transfers each row of data from
the Sampler object to a sub-application simulation. Within the sub-application the `parameters` listed
in this sub-block replace the values in the sub-application. This substitution occurs using
the aforementioned SamplerReciever object that exists in the Controls block of the sub-application
input file. The receiver on the sub-application accepts the parameter names and values from the
SamplerParameterTransfer object on the main application.

The "results" sub-block serves the second purpose by transferring the quantities of interest back to
the main application. In this case those quantities are postprocessors on the sub-application that
compute $T_{avg}$ and $q_{left}$. These computed results are transferred from the sub-application to
a [StochasticReporter.md] object (as discussed next).

!listing examples/parameter_study/main.i block=Transfers

### Reporters Block

The [Reporters](syntax/Reporters/index.md) block, as mentioned above,
is used to collect the stochastic results. The "results" sub-block instantiates a
[StochasticReporter.md] object, which is designed for this purpose. The resulting object will
contain a vector for each of the quantities of interest: $T_{avg}$ and $q_{left}$.

The [StatisticsReporter.md] object is designed to compute multiple statistics and confidence intervals for each
of the input vectors. In this case it computes the mean and standard deviation for $T_{avg}$ and $q_{left}$ vectors
in the "results" object as well as the 5% and 95% confidence level intervals. Please
refer to the documentation for the [StatisticsReporter.md] object for further
documentation and capabilities for this object.

!listing examples/parameter_study/main.i block=Reporters

### Outputs Block

The [Outputs](syntax/Outputs/index.md) block enables the output of the Reporter data
using JSON files, see [JSONOutput.md] for more details.

!listing examples/parameter_study/main.i block=Outputs

## Stochastic Results id=results

The input file described in the previous section can be run with the following command:

```language=sh
mpiexec -n <n> ../../stochastic_tools-opt -i main.i
```

The `<n>` is the number of processors to be run with. It is recommended to use a number
greater than 8 to finish the calculation in a reasonable amount of time.
The result will be an output of a `main_out.json*` file (one for each processor). If only
one file is desired use the option `Reporters/results/parallel_type=ROOT`.

### Quantity of Interest Distributions

The resulting distributions are for the quantities of interest: $T_{avg}$ and $q_{left}$ are presented
in [results_T_avg] and [results_q_left]. These plots were generated using the following commands:

```language=sh
python ../../python/make_histogram.py main_out.json* -v results:T_avg:value --xlabel 'Average Temperature'
python ../../python/make_histogram.py main_out.json* -v results:q_left:value --xlabel 'Flux'
```

!media stochastic_tools/parameter_study/tavg_hist.png
       id=results_T_avg
       caption=Resulting distribution of quantity of interest: $T_{avg}$.
       style=width:50%;margin-left:auto;margin-right:auto;halign:center

!media stochastic_tools/parameter_study/flux_hist.png
       id=results_q_left
       caption=Resulting distribution of quantity of interest: $q_{left}$.
       style=width:50%;margin-left:auto;margin-right:auto;halign:center

### Statistics

[stats] includes the computed statistics and confidence level intervals as computed
by the StatisticsReporter object for the example heat conduction problem with 5000 samples.
This table was generated in markdown format using the following command:

```language=sh
python ../../python/visualize_statistics.py main_out.json --markdown-table \
--names '{"results_results:T_avg:value":"$T_{avg}$","results_results:q_left:value":"$q_{left}$"}' \
--stat-names '{"MEAN":"Mean","STDDEV":"Standard Deviation"}'
```

!table id=stats
| Values                      | Mean                 | Standard Deviation   |
|:----------------------------|:---------------------|:---------------------|
| $T_{avg}$ (5.0%, 95.0%) CI  | 199.3 (198.1, 200.5) | 51.55 (50.67, 52.41) |
| $q_{left}$ (5.0%, 95.0%) CI | 179.3 (177.4, 181.2) | 82.71 (81.24, 84.19) |

These statistics can also be visualized with a bar plot:

```language=sh
python ../../python/visualize_statistics.py main_out.json --bar-plot \
--names '{"results_results:T_avg:value":"Average Temperature","results_results:q_left:value":"Flux"}' \
--stat-names '{"MEAN":"Mean","STDDEV":"Standard Deviation"}'
```

!media stochastic_tools/parameter_study/stats_bar.png

## Time Dependent and Vector Quantities

Often, it is useful to perform parameter studies with quantities that are time-dependent. This section
will show how to modify the previous inputs for quantities that are time-dependent or possibly vectors.

### Sub-application Input Modifications

The only change to the sub-app input is the addition of a [LineValueSampler.md] vector-postprocessor
that computes the spatial distribution of the temperature.

!listing examples/parameter_study/diffusion_time.i block=VectorPostprocessors

### Main Input Modifications

The two significant modifications to the main input is the change to a [SamplerTransientMultiApp.md] and
the addition of the [Executioner](syntax/Executioner/index.md) block. The combination of these two blocks
will run the sampled sub-applications in tandem with the time-step defined in the main input. The main
application in this case will "drive" the overall simulation (and does support sub-cycling). Here, we
use the same time stepping as in the sub-application.

!listing examples/parameter_study/main_time.i block=MultiApps Executioner

We will also add the transfer for the [LineValueSampler.md] included in the sub input, while also computing
statistics and transferring the x-coord values. Note that the [ConstantReporter.md] is just a place holder
for the [MultiAppReporterTransfer.md] to transfer the x-coord data into.

!listing examples/parameter_study/main_time.i block=Transfers Reporters

### Statistics

The output (`main_time_out.json`) will now have statistics for all three quantities for each time step. We can
report this data using the following:

```language=sh
python ../../python/visualize_statistics.py main_time_out.json  --markdown-table \
--values results_results:T_avg:value results_results:q_left:value \
--names '{"results_results:T_avg:value":"Average Temperature","results_results:q_left:value":"Flux"}' \
--stats MEAN STDDEV --stat-names '{"MEAN":"Mean","STDDEV":"Standard Deviation"}'
```

!table
| Values                               |   Time | Mean                 | Standard Deviation   |
|:-------------------------------------|-------:|:---------------------|:---------------------|
| Average Temperature (5.0%, 95.0%) CI |   0.25 | 228 (227.3, 228.7)   | 29.38 (28.96, 29.78) |
|                                      |   0.5  | 209.9 (209, 210.9)   | 41.58 (40.96, 42.19) |
|                                      |   0.75 | 202.5 (201.4, 203.7) | 47.98 (47.21, 48.74) |
|                                      |   1    | 199.3 (198.1, 200.5) | 51.55 (50.67, 52.41) |
| Flux (5.0%, 95.0%) CI                |   0.25 | 337.4 (335.4, 339.3) | 84 (82.56, 85.43)    |
|                                      |   0.5  | 214.8 (213.1, 216.4) | 70.35 (69.23, 71.46) |
|                                      |   0.75 | 188.6 (186.8, 190.3) | 76.56 (75.29, 77.82) |
|                                      |   1    | 179.3 (177.4, 181.2) | 82.71 (81.24, 84.19) |

```language=sh
python ../../python/visualize_statistics.py main_time_out.json --line-plot \
--values results_results:T_avg:value results_results:q_left:value \
--names '{"results_results:T_avg:value":"Average Temperature","results_results:q_left:value":"Flux"}' \
--stats MEAN STDDEV --stat-names '{"MEAN":"Mean","STDDEV":"Standard Deviation"}'
```

!media stochastic_tools/parameter_study/stats_time_timeline.png

```language=sh
python ../../python/visualize_statistics.py main_time_out.json --line-plot \
--values results_results:T_vec:T --names '{"results_results:T_vec:T":"Temperature"}' \
--stats MEAN STDDEV --stat-names '{"MEAN":"Mean","STDDEV":"Standard Deviation"}' \
--xvalue x
```

!media stochastic_tools/parameter_study/stats_time_line.png

## Time Dependent Quatities with AccumulateReporter

You might find that using [SamplerTransientMultiApp.md], like in the previous sections, is a bit
restrictive. For instance, the time steps in the sub-app input must be defined by the steps in the main input.
This restricts the use of [TimeSteppers](syntax/Executioner/TimeStepper/index.md) like [IterationAdaptiveDT.md]
and sub-cycling sub-sub-apps becomes difficult. Also, [SamplerTransientMultiApp.md] does not support
`batch-reset` mode, so there isn't a memory-efficient way of sampling with [MultiAppSamplerControl.md].

An alternative to using [SamplerTransientMultiApp.md] is to leverage [AccumulateReporter.md], which accumulates
postprocessors/vector-postprocessor/reporters into vector where each element is the value at a certain
time-step.

### Sub-application Input Modifications

For this example, we will accumulate the `T_avg` and `q_left` postprocessors. We can accumulate `T_vec` from
the previous section, but [StatisticsReporter.md] does not yet support vector-of-vector quantities.

!listing examples/parameter_study/diffusion_vector.i block=Reporters

### Main Input Modifications

There are no major modifications to the main input from the first section. We only need to replace the transferred
values from `*:value` to `acc:*:value`.

!listing examples/parameter_study/main_vector.i block=Transfers Reporters

### Statistics

The output (`main_vector_out.json`) has the statistics for each quantity represented by a vector. All of the
previous results can be reproduced with this output. One fancy thing we can do with this new output:

```language=sh
python ../../python/visualize_statistics.py main_vector_out.json --line-plot \
--names '{"results_results:acc:T_avg:value":"Average Temperature","results_results:acc:q_left:value":"Flux"}' \
--xvalue results_results:acc:T_avg:value \
--stats MEAN --stat-names '{"MEAN":"Mean"}'
```

!media stochastic_tools/parameter_study/mean_vector_line.png

```language=sh
python ../../python/visualize_statistics.py main_vector_out.json --line-plot \
--names '{"results_results:acc:T_avg:value":"Average Temperature","results_results:acc:q_left:value":"Flux"}' \
--xvalue results_results:acc:T_avg:value \
--stats STDDEV --stat-names '{"STDDEV":"Standard Deviation"}'
```

!media stochastic_tools/parameter_study/stddev_vector_line.png
