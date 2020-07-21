# Parameter Study

A parameter study is defined here as performing a simulation with varying uncertain
parameters and computing quantities of interest for each set of uncertain parameters.
This example is a more detailed example of what is presented in [examples/monte_carlo.md], which
is also a parameter study.

## Problem Statement

To demonstrate how to perform a parameter study the transient heat equation will be used.

!equation id=diffusion-strong
\frac{\partial T}{\partial t} - \nabla\cdot\gamma\nabla T + s = 0,

where $T$ is temperature, $t$ is time, $\gamma$ is diffusivity, and $s$ is a heat source term. For
this example this problem is shall be solved on a 2D unit square domain. The left side is subjected
to a Dirichlet condition $T=T_0$. The right side of the domain is subject to a Neumann condition
$\gamma\nabla T \cdot \hat{n} = q_0$, where $\hat{n}$ is the outward facing normal vector from the
boundary.

The boundary conditions $T_0$ and $q_0$ as well as the diffusivity ($\gamma$) and the source
term $s$ are assumed to be known. However, these parameters have some associated uncertainty.
The effect of the uncertainty on the average temperature and heat flux on the left boundary after
one second of simulation time is desired, defined as $T_{avg} = \bar{T}(t=1)$ and
$q_{left} = q(t=1,x=0)$.

## Heat Equation Problem

The first step in performing a parameter study is to create an input file that solves your problems
without uncertain parameters. For the example problem this will be done assuming $\gamma=1$,
$s=1$, $T_0=-10$, and $q_0=-100$.

The complete input file for this problem is provided in [diffusion]. The only item
that is atypical from a MOOSE simulation input file is the existence of the `Controls` block, which
here simply creates a [SamplerReceiver.md] object. This block is required for the parameter study,
but shall be discussed in [parameter_study.md#transfers] section.

!listing parameter_study/diffusion.i id=diffusion
         caption=Complete input file for example heat equation problem to be used in parameter study.

Executing this file using the StochasticToolsApp can be done as follows

```
cd moose/modules/stochastic_tools/examples/parameter_study
../../stochastic_tools-opt -i diffusion.i
```

The simulation will perform four timesteps and should result in $T_{avg}=-5.12$ and $q_{left}=-7.77$.


## Master Input

To perform a parameter study an input file that will drive the stochastic simulations is required.
This file will be referred to as the master input file and is responsible for defining the
uncertain parameters, running the stochastic simulations, and collecting the stochastic results. The
following sub-sections will step through each portion of the master file to explain the purpose. The
complete analysis is discussed in [parameter_study.md#results].

### StochasticTools Block

The [StochasticTools](syntax/StochasticTools/index.md) block sets up the master file to be a
driver for stochastic simulations but itself is not performing any sort of simulation. Setting up
a master without a simulation itself is the default behavior for this block, as such it
is empty in this example.

!listing examples/parameter_study/master.i block=StochasticTools

### Distributions and Samplers Blocks

The [Distributions](syntax/Distributions/index.md) block defines the statistical distribution
for each of the uncertain parameters ($T_0$, $q_0$, $\gamma$, and $s$) to be defined. For this
example, the diffusivity ($\gamma$) is defined by a uniform distribution, the flux ($q_0$) by
a three-parameter Weibull, and $T_0$ and $s$ a normal distribution.

The [Samplers](syntax/Samplers/index.md) block defines how the distributions shall be sampled for
the parameter study. In this case a Latin hypercube sampling strategy is employed, where each
distribution is sampled over the complete range with ten bins each. A total of 500 samples are
being used.

!listing examples/parameter_study/master.i block=Samplers

For this problem the Sampler object is setup to run in "batch-restore" mode, which is a mode of
operation for memory efficient creation of sub-applications. Please see
[stochastic_tools/batch_mode.md] for more information.

The input distributions for each of these four terms are included in [sampler_gamma]--[sampler_s]
for 5000 samples.

!listing examples/parameter_study/master.i block=Distributions

!plot histogram filename=stochastic_tools/examples/parameter_study/gold/master_out_samples_0002.csv
                vectors=hypercube_0
                xlabel=Diffusivity
                id=sampler_gamma
                legend=False
                caption=Input distribution of diffusivity ($\gamma$).

!plot histogram filename=stochastic_tools/examples/parameter_study/gold/master_out_samples_0002.csv
                vectors=hypercube_1
                xlabel=Flux
                id=sampler_q_0
                legend=False
                caption=Input distribution of flux boundary condition ($q_0$).

!plot histogram filename=stochastic_tools/examples/parameter_study/gold/master_out_samples_0002.csv
                vectors=hypercube_2
                xlabel=Temperature
                id=sampler_T_0
                legend=False
                caption=Input distribution of temperature boundary condition ($T_0$).

!plot histogram filename=stochastic_tools/examples/parameter_study/gold/master_out_samples_0002.csv
                vectors=hypercube_3
                xlabel=Source
                id=sampler_s
                legend=False
                caption=Input distribution of source term ($s$).

### Transfers Block id=transfers

The [Transfers](syntax/Transfers/index.md) block serves two purposes. First, the "parameters"
sub-block instantiates a [SamplerParameterTransfer.md] object that transfers each row of data from
the Sampler object to a sub-application simulation. Within the sub-application the parameters listed
in the in this sub-block replace the values in the sub-application. This substitution occurs using
the aforementioned SamplerReciever object that exists in the Controls block of the sub-application
input file. The receiver on the sub-application accepts the parameter names and values from the
SamplerParameterTransfer object on the master application.

The results sub-blocks, serves the second purpose by transferring the quantities of interest back to
the master application. In this cases those quantities are postprocessors on the sub-application that
compute $T_{avg}$ and $q_{left}$. These computed results are transferred from the sub-application to
a VectorPostprocessor object (as discussed next).

!listing examples/parameter_study/master.i block=Transfers

### VectorPostprocessor Block

The [VectorPostprocessor](syntax/VectorPostprocessors/index.md) block, as mentioned above,
is used to collect the stochastic results. The "results" sub-block instantiates a
[StochasticResults.md] object, which is designed for this purpose. The resulting object will
contain a vector for each of the quantities of interest: $T_{avg}$ and $q_{left}$.

The "samples" sub-block simply is used as a means to output the sample data, it is used here
for presenting the results in the next section.

This Statistics object is designed to compute multiple statistics and confidence intervals for each
of the input vectors. In this case it computes the mean for $T_{avg}$ and $q_{left}$ vectors
in the "results" object as well as the 5% and 95% confidence level intervals. Please
refer to the documentation for the [vectorpostprocessors/Statistics.md] object for further
documentation and capabilities for this object.

!listing examples/parameter_study/master.i block=VectorPostprocessors

### Outputs Block

The [Outputs](syntax/Outputs/index.md) block enables the output of the VectorPostprocessor data
using the comma separated files.

!listing examples/parameter_study/master.i block=Outputs

## Stochastic Results id=results

### Quantity of Interest Distribtions

The resulting distributions are for the quantities of interest: $T_{avg}$ and $q_{left}$ are presented
in [results_T_avg] and [results_q_left].

!plot histogram filename=stochastic_tools/examples/parameter_study/gold/master_out_results_0002.csv
                vectors=results:T_avg
                bins=20
                xlabel=Average Temperature
                id=results_T_avg
                caption=Resulting distribution of quantity of interest: $T_{avg}$.


!plot histogram filename=stochastic_tools/examples/parameter_study/gold/master_out_results_0002.csv
                vectors=results:q_left
                bins=20
                xlabel=Flux
                id=results_q_left
                caption=Resulting distribution of quantity of interest: $q_{left}$.

#### Statistics

[stats] includes the computed statistics and confidence level intervals as output
by the VectorPostprocessor object for the example heat conduction problem with 5000 samples. The
results of which can be written as:

$\overline{T}_{avg} = -22.52,\,95\%\, CI[-22.76, -22.29]$

$\overline{q}_{left} = -66.65,\,95\%\, CI[-66.34, -65.97]$

!listing id=stats
         caption=Resulting statistics and confidence level intervals for the computed quantities
                 of interest.
stat_type,results_results:T_avg,results_results:q_left
3,-22.524612297114,-66.65304703336
3.05,-22.757770795825,-67.338628188985
3.95,-22.294482681111,-65.972800237941
