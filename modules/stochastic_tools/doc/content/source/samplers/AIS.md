# Adaptive Importance Sampling (AIS)

!syntax description /Samplers/AIS

## Description

AIS is used for estimating small failure probabilities ($P_f$) using computationally expensive
finite element models efficiently. When implemented correctly, AIS can result in
2-3 orders of mangitude reduction to the number of calls to the model when compared to
 crude Monte Carlo simulations. For example, if crude Monte Carlo requires 1e6 calls
to the model to estimate $P_f$ with a coefficient of variation (COV) of 10 percent, AIS could
require only about 1e4 calls to the model for the same COV. AIS has two phases:
 (1) a learning phase where the AIS algorithm learns the importance distribution of
 input parameters likely to cause model failure; and (2) a sampling phase to sample
from the learned importance distribution. The version of AIS proposed by [!cite](Au1999) is
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
The use of an independent Normal distribution is different from the work by [!cite](Au1999),
who use a multi-dimensional kernel density distribution. However, experience suggests that
a Normal distribution is more robust under a wide variety of cases. Once the samples from the
importance distribution are obtained, equation (2) and equation (19) in [!cite](Au1999)
can be used for estimating the $P_f$ and COV, respectively.

## Interaction with the `AdaptiveMonteCarloDecision` class

The AIS algorithm has a proposal step and a decision-making step on whether or not to
 accept the proposed sample. While the `AIS` class proposes a new sample, the `AdaptiveMonteCarloDecision` class
is used for decision-making. Please refer to [AdaptiveMonteCarloDecision](AdaptiveMonteCarloDecision.md)
for more details.

## Example Input Syntax

The input file for using the AIS algorithm is somewhat similar to the other sampler
 classes except for three differences. First, the `Samplers` block is presented below:

!listing modules/stochastic_tools/test/tests/samplers/AIS/ais.i block=Samplers

where, `proposal_std` are proposal standard deviations (in Standard Normal space) used during the learning phase,
 `output_limit` is the limiting value greater than which is characterized as model failure
 , `num_samples_train` is the number of samples to learn the importance distribution,
 `std_factor` is the factor multiplied to the standard deviation of the importance samples
while characterizing the importance distribution, `use_absolute_value` is used this when failure is defined as a non-exceedance rather than an exceedance. `inputs_reporter` and `output_reporter` are the
reporter values which transfer information between the `AIS` sampler and the
`AdaptiveMonteCarloDecision` reporter.

Second, the `Reporters` block is presented below with the `AdaptiveMonteCarloDecision` reporter:

!listing modules/stochastic_tools/test/tests/samplers/AIS/ais.i block=Reporters

where, output reporter and the inputs reporter are both initialized.

Third, the `Executioner` block is presented below:

!listing modules/stochastic_tools/test/tests/samplers/AIS/ais.i block=Executioner

where, it is noticed that unlike some other sampler classes, the `type` is transient.
 `num_steps` in the above code block are the total number of samples required; that is
 learning phase samples plus importance sampling phase samples.

## Output format

The `AIS` sampler can output a csv or a json file. Such a file will contain fields
for all the input parameters and an additional field containing zeros or ones which indicate
model failure. The first `num_samples_train` rows are the samples during the learning
phase and should not be used for estimating the $P_f$ or COV. The next `num_steps - num_samples_train`
are the required importance samples used for estimating $P_f$ or COV.

!syntax parameters /Samplers/AIS

!syntax inputs /Samplers/AIS

!syntax children /Samplers/AIS
