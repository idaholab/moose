# AISActiveLearning (AISActiveLearning)

!syntax description /Samplers/AISActiveLearning

## Description

As stated in [AdaptiveImportanceSampler](AdaptiveImportanceSampler.md), there are two steps in an
Adaptive Importance Sampler (AIS): (1) usage of a Markov chain Monte Carlo (MCMC) sampler to learn the
importance region; and (2) regular Monte Carlo sampling from the importance region for variance reduction
when estimating a quantity of interest (QoI; like the failure probability). However, each MCMC or Monte Carlo
sample is associated with a full model evaluation in the traditional AIS. While AIS considerably reduces the
computational cost for estimating a QoI compared to a regular Monte Carlo sampler, even more computational gains
are obtained by integrating active learnign into AIS.

Active learning is based on the Gaussian process (GP) surrogate; see [ActiveLearningGaussianProcess](ActiveLearningGaussianProcess.md).
Once the GP is trained with a few outputs from the full model, for every new input sample from either MCMC or
Monte Carlo, a GP prediction is first made along with the prediction uncertainty. This prediction and the
uncertainty are used to assess the prediction quality with the aid of active learning functions. If the GP
prediction quality is good, we simply move onto a new input sample. Otherwise, we call the full model and re-train the GP with including the new sample in the training set to improve the future predictive performance.

## Interaction between `AISActiveLearning`, `ActiveLearningGPDecision`, and `AdaptiveMonteCarloDecision`

Active learning in AIS primarily relies on three objects: `AISActiveLearning`,
[ActiveLearningGPDecision](ActiveLearningGPDecision.md), and [AdaptiveMonteCarloDecision](AdaptiveMonteCarloDecision.md).
The interaction between these objects is presented in [!ref](alais_sch) and is further discussed below.

!media ActiveLearning_consolidated.svg style=width:75%; id=alais_sch caption=Schematic of active learning in Adaptive Importance Sampling. The interaction between the three objects, `AISActiveLearning`, `ActiveLearningGPDecision`, and `AdaptiveMonteCarloDecision`, is presented.

The interaction between these three objects is straightforward to understand. Once the GP is trained,
`AISActiveLearning` proposes a new input sample, either using MCMC or Monte Carlo.
By default, [ActiveLearningGPDecision](ActiveLearningGPDecision.md) uses a GP to predict the model output
and also assesses the prediction quality.

The details on how the GP is initially trained and subsequently re-trained are discussed in [ActiveLearningGPDecision](ActiveLearningGPDecision.md).

!alert note title=Start of the importance sampling procedure
The importance sampling using MCMC does not start until the GP initial training is finished.

## Input file syntax

Once the interaction between the three objects is understood, the input file syntax is easy to follow.

The `AISActiveLearning` samplers block is largely similar to [AdaptiveImportanceSampler](AdaptiveImportanceSampler.md).
One difference is that the [!param](/Samplers/AISActiveLearning/flag_sample) parameter
is requested to identify whether the GP prediction is good or bad. This dictates the next input proposal.

!listing modules/stochastic_tools/test/tests/reporters/AISActiveLearning/ais_al.i block=Samplers

The `ActiveLearningGPDecision` reporters block is the same as active learning in Monte Carlo sampling. See
[ActiveLearningGPDecision](ActiveLearningGPDecision.md) for the details.

!listing modules/stochastic_tools/test/tests/reporters/AISActiveLearning/ais_al.i block=Reporters/conditional

The `AdaptiveMonteCarloDecision` reporters block is also largely similar to [AdaptiveImportanceSampler](AdaptiveImportanceSampler.md).
One difference is, instead of using the full model outputs, the GP mean prediction is used.

!listing modules/stochastic_tools/test/tests/reporters/AISActiveLearning/ais_al.i block=Reporters/adaptive_MC

## Adaptive importance statistics reporter

The [AdaptiveImportanceStats](AdaptiveImportanceStats.md) can also be used in AIS with active learning.
The syntax is show below.

!listing modules/stochastic_tools/test/tests/reporters/AISActiveLearning/ais_al.i block=Reporters/ais_stats

## Output format

!syntax parameters /Samplers/AISActiveLearning

!syntax inputs /Samplers/AISActiveLearning

!syntax children /Samplers/AISActiveLearning