# Adaptive Importance Sampling (AIS)

!syntax description /Samplers/AdaptiveImportance

## Description

AIS is used for estimating small failure probabilities ($P_f$) using computationally expensive
finite element models efficiently. When implemented correctly, AIS can result in
2-3 orders of mangitude reduction to the number of calls to the model when compared to
 crude Monte Carlo simulations. For example, if crude Monte Carlo requires 1e6 calls
to the model to estimate $P_f$ with a coefficient of variation (COV) of 10 percent, AIS could
require only about 1e4 calls to the model for the same COV. AIS has two phases:
 (1) a learning phase where the AIS algorithm learns the importance distribution of
 input parameters likely to cause model failure; and (2) a sampling phase to sample
from the learned importance distribution. The version of AIS proposed by [!cite](au1999new) is
implemented in MOOSE with a slight modification. Instead of using kernel density to
 characterize the importance distribution, a Normal density is used due to its simplicity
 and robustness.

## Brief algorithmic details

For learning the importance distribution of the input parameters, a Markov Chain
Monte Carlo algorithm, specifically the Metropolis algorithm, is used. The user supplies
 a `initial_values` vector of input parameters that would result in a model failure.
 Then, a Markov Chain is initiated to sufficiently sample from the failure region.
 The Markov Chain operates in Standard Normal space where all the input parameters
are transformed to have a Normal distribution with mean 0 and standard deviation 1.
Centered around the current sample ($\mathbf{x}$), a new sample ($\mathbf{x}^*$) is proposed considering the
proposal distribution to be Normal and an acceptance ratio is computed:

\begin{equation}
\label{eqn:ais_1}
\alpha = \frac{N(\mathbf{x}^*)}{N(\mathbf{x})}
\end{equation}

where, $N(.)$ is a standard Normal distribution. The proposed sample is then accepted
with probability $\alpha$. In this manner, sufficient samples are generated from the
failure region.

Once enough samples that result in model failure have been simulated, an imporatance
 distribution is fit to these samples. A Normal distribution is fit to each
input parameter independently and sampling from this importance distribution is made.
The use of an independent Normal distribution is different from the work by [!cite](au1999new),
who use a multi-dimensional kernel density distribution. However, experience suggests that
a Normal distribution is more robust under a wide variety of cases. Once the samples from the
importance distribution are obtained, equation (2) and equation (19) in [!cite](au1999new)
can be used for estimating the $P_f$ and COV, respectively. Figure [ais_sch] presents
 a schematic of the AIS method.

!media Adaptive_Importance_Sampler.svg style=width:50%; id=ais_sch caption=Schematic of the AIS method

## Interaction with the `AdaptiveMonteCarloDecision` class

The AIS algorithm has a proposal step and a decision-making step on whether or not to
 accept the proposed sample. While the `AIS` class proposes a new sample, the `AdaptiveMonteCarloDecision` class
is used for decision-making. Please refer to [AdaptiveMonteCarloDecision](AdaptiveMonteCarloDecision.md)
for more details.

## Example Input Syntax

The input file for using the AIS algorithm is somewhat similar to the other sampler
 classes except for three differences. First, the `Samplers` block is presented below:

!listing modules/stochastic_tools/test/tests/samplers/AdaptiveImportanceSampler/ais.i block=Samplers

where, `proposal_std` are proposal standard deviations (in Standard Normal space) used during the learning phase,
 `output_limit` is the limiting value greater than which is characterized as model failure
 , `num_samples_train` is the number of samples to learn the importance distribution,
 `num_importance_sampling_steps` is the number of importance sampling steps (after the importance distribution has been trained),
 `std_factor` is the factor multiplied to the standard deviation of the importance samples
while characterizing the importance distribution, `use_absolute_value` can be set to true when failure is defined as a non-exceedance rather than an exceedance. `inputs_reporter` and `output_reporter` are the
reporter values which transfer information between the `AIS` sampler and the
`AdaptiveMonteCarloDecision` reporter.

Second, the `Reporters` block is presented below with the `AdaptiveMonteCarloDecision` reporter:

!listing modules/stochastic_tools/test/tests/samplers/AdaptiveImportanceSampler/ais.i block=Reporters

where, output reporter and the inputs reporter are both initialized.

Third, the `Executioner` block is presented below:

!listing modules/stochastic_tools/test/tests/samplers/AdaptiveImportanceSampler/ais.i block=Executioner

where it is noticed that unlike some other sampler classes, the `type` is transient.
The number of time steps is automatically determined based on `num_samples_train` and `num_importance_sampling_steps`.

## Output format

The `AIS` sampler can output a csv or a json file. However, a json output is a preferred output format
for post-processing sampler data from adaptive Monte Carlo algorithms. The json file
consists of "inputs" and "output_required" corresponding to each "time_step". "inputs" are
the uncertain input parameters to the model, "output_required" is an indicator function
on whether or not the model output exceeded `output_limit`, and "time_step" is the
sample index. The first `num_samples_train` time steps are the samples during the
learning phase and should not be used for estimating the $P_f$ or COV. The next
`num_steps - num_samples_train` are the required importance samples used for estimating $P_f$ or COV.

## Failure probability and COV

Statistics such as the $P_f$ and COV can be computed using the [AdaptiveImportanceStats](AdaptiveImportanceStats.md)
Reporter object.

!syntax parameters /Samplers/AdaptiveImportance

!syntax inputs /Samplers/AdaptiveImportance

!syntax children /Samplers/AdaptiveImportance
