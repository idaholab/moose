# Parallel Subset Simulation (PSS)

!syntax description /Samplers/ParallelSubsetSimulation

## Description

PSS is used for efficiently estimating small failure probabilities and for conducting
optimization under uncertainties when dealing with computationally expensive numerical models.
It can result in 2-3 orders of magnitude fewer calls to the numerical model when estimating
failure probabilities or conducting optimization under uncertainty compared to standard
Monte Carlo or Latin Hypercube Sampling. PSS works by creating intermediate failure
thresholds to efficiently transition from sampling from the nominal input space to sampling
from input spaces which optimize the numerical model output. Use of Markov Chain Monte Carlo (MCMC)
is key to the PSS algorithm. In fact, PSS uses hundreds of Markov Chains to efficiently
propagate to the regions of the input space that are most important with respect
to optimizing the model output. Since these Markov chains are independent of each other,
they can be run in parallel on a different set of processors. It should be noted that
parallelization can only be achieved across Markov chains and not within a chain.
That is, a given set of processors should completely run an entire chain composed
of a certain number of samples (this number is usually 10). Coupled with several
Markov chains and massively parallel computing, PSS may require only 1E1 to 1E2
order of magnitude numerical model evaluations for accurately estimating low failure probabilities or
for conducting optimization studies.

## Brief algorithmic details

Exhaustive algorithmic details of PSS have been presented in [!cite](au2001pss). Only
a brief description of the algorithm is presented here. To efficiently sample from
regions of input parameter spaces that are crucial for optimization and failure
evaluation of the model output, PSS creates intermediate failure thresholds defined
through the equation:

\begin{equation}
\label{eqn:pss_1}
P_f= P_1~\prod_{i=2}^{N_s}~P_{i|i-1}
\end{equation}

where, $P_f$ is the failure probability of interest, $P_1$ and $P_{i|i-1}~~(i \in \{2,\dots,N_s\})$
are the intermediate failure probabilities defining the intermediate failure thresholds, and
$N_s$ is the number of subsets. Through the intermediate failure probabilities $P_1$ and $P_{i|i-1}$
PSS creates intermediate failure thresholds that allow to efficiently transition to sampling
from input spaces which cause numerical model failure or numerical model optimal output. These
intermediate failure thresholds are created with the aid of numerous Markov chains.

In practice, SS is implemented in the following manner. If $N$ is the total number
of numerical model evaluations, each subset will have $M=N/N_s$ samples. An MCS is first used to generate $M$ samples.
If the intermediate failure probabilities (except $P_{N_s|N_{s-1}}$)
are all fixed to 0.1, then the first intermediate failure threshold ${F}_1$
is estimated as the $90^{\textrm{th}}$ percentile value of all the $M$ numerical model outputs.
The outputs that do not exceed $\mathcal{F}_1$ constitute Subset 1. To determine
the next failure threshold $\mathcal{F}_2$, conditional samples should be generated
such that the numerical model outputs always exceed $\mathcal{F}_1$. An MCMC method---in particular,
a component-wise Metropolis method---is used to estimate $\mathcal{F}_2$ by simulating
numerous Markov chains. From the $M$ MCS samples in the first subset, those that exceeded
the threshold $\mathcal{F}_1$ are used as seeds (or starting values) for these Markov chains.
If $M$ samples need to be simulated such that the outputs exceed $\mathcal{F}_1$,
there will be $0.1~M$ Markov chains, with each chain simulating $1/0.1$ samples.
In general, if the intermediate failure probabilities (except $P_{N_s|N_{s-1}}$) are
fixed to $p_o$ instead of 0.1, there will be $p_o~M$ Markov chains, with each chain
simulating $1/p_o$ samples. Once $M$ samples are generated from $p_o~M$
Markov chains, the second intermediate failure threshold ${F}_2$ is the $(1-p_o) \times 100$
percentile value of all the samples' outputs. Samples between ${F}_1$ and ${F}_2$ comprise the second subset.
A similar procedure of simulating $(p_o~M)$ Markov chains is repeated for determining
the subsequent failure thresholds until the final required failure threshold $F$ is reached.
More details on the practical implementation of the PSS method are presented in [!cite](li2016pss).
[!ref](pss_sch) presents a schematic of the PSS method.

!media Parallel_Subset_Simulation_Sampler.svg style=width:50%; id=pss_sch caption=Schematic of the PSS method

## Parallelization of Parallel Subset Simulation: An example

Since PSS relies of numerous Markov chains, it can be parallelized. Each Markov chain
can be run independently on separate sets of processors in parallel to other Markov chains.
For optimal PSS performance, it is recommended that the number of samples evaluated
by each processor is a multiple of 10. For example, consider that we are interested
in evaluating the small failure probability (of the order $1E-4$) of a numerical model. For this,
we select 4 subsets with 10,000 samples per subset. In total, there will be 40,000
numerical model evaluations. If we use 1000 processors with $0.1$ as the intermediate failure
probability, there will be 1000 Markov chains per subset with each chain making 10 numerical
model evaluations. Therefore each processor evaluates the numerical model 10 times per subset.
With 4 subsets, each processor evaluates the numerical model 40 times. In contrast, if we use
only 100 processors, each processor solves the numerical model 100 times per subset.
With 4 subsets, each processor solves the numerical model 400 times in total.

Alternatively, if we use 2000 processors, a unique set of two processors jointly
solves the numerical model 10 times per subset and 40 times overall. Beyond 1000 processors,
the number of numerical model solves per processor cannot be reduced further. This is due to
the fact that parallelization can only be achieved across Markov chains and not within
a chain. Each processor should solve the numerical model a minimum of 10 times per subset in this
example because the number of samples in a Markov chain are $1/0.1$ with the intermediate
failure probability fixed to 0.1.

!alert note title=Markov chain proposals in MOOSE
In MOOSE, currently, Markov chain proposals are made by transforming all the inputs
  into a standard Normal space and then adding a Gaussian noise to a previously
  accepted sample with a unit standard deviation. Expanding this proposal scheme to
  incorporate other MCMC algorithms is possible and would be of interest in the future.

## Interaction with the `AdaptiveMonteCarloDecision` class

The PSS algorithm is an adaptive algorithm and it has a proposal step for all the
Markov chains and a decision-making step on whether or not to accept the proposed samples.
While the `ParallelSubsetSimulation` class proposes new samples across multiple Markov
chains, the `AdaptiveMonteCarloDecision` class
is used for decision-making. Please refer to [AdaptiveMonteCarloDecision](AdaptiveMonteCarloDecision.md)
for more details.

## Example Input Syntax

The input file for using the PSS algorithm is somewhat similar to the other sampler
 classes except for three differences. First, the `Samplers` block is presented below:

!listing modules/stochastic_tools/test/tests/samplers/ParallelSubsetSimulation/pss.i block=Samplers

where, `num_samplessub` is the number of samples per subset and `num_subsets` is the number of subsets.
`inputs_reporter` and `output_reporter` are the reporter values which transfer
information between the `ParallelSubsetSimulation` sampler and the `AdaptiveMonteCarloDecision` reporter. There is an optional input parameter `subset_probability` which has been defaulted to
`0.1`, meaning that there are $1/0.1$ samples per Markov chain. This can, however, be
changed as per the user preference. `num_parallel_chains` is also an optional parameter
that explicitly specifies the number of Markov chains to be run in parallel per subset.
If `num_parallel_chains` is not specified, the number of parallel Markov chains per subset
will be equal to the number of processors. Besides, `use_absolute_value` can be set to true
when failure is defined as a non-exceedance rather than an exceedance.

Second, the `Reporters` block is presented below with the `AdaptiveMonteCarloDecision` reporter:

!listing modules/stochastic_tools/test/tests/samplers/ParallelSubsetSimulation/pss.i block=Reporters

where, the output and input reporters are both initialized.

Third, the `Executioner` block is presented below:

!listing modules/stochastic_tools/test/tests/samplers/ParallelSubsetSimulation/pss.i block=Executioner

where it is noticed that unlike some other sampler classes, the `type` is transient.
The number of time steps is automatically determined based on `num_samplessub`, `num_subsets` and the number of processors used.


## Output format

The `ParallelSubsetSimulation` sampler can output a csv or a json file. However, a json output is a preferred output format
for post-processing sampler data from adaptive Monte Carlo algorithms. The json file
consists of "inputs" and "output_required" corresponding to each "time_step" across
all the processors.

!syntax parameters /Samplers/ParallelSubsetSimulation

!syntax inputs /Samplers/ParallelSubsetSimulation

!syntax children /Samplers/ParallelSubsetSimulation
