# AdaptiveMonteCarloDecision

!syntax description /Reporters/AdaptiveMonteCarloDecision

## Description

Adaptive Monte Carlo algorithms, generally speaking, work by proposing the next set
of input parameters, performing a model evaluation, and deciding whether or not to
accept the proposed input parameters. Classes derived from the [Sampler](Sampler.md)
class help propose the next set of input parameters and classes derived from the
MultiApp system to evaluate the model. However, an additional
class is needed to make the decision on whether or not to accept the next input parameters
proposed by the sampler class. To this end, `AdaptiveMonteCarloDecision` will help
make such decisions. `AdaptiveMonteCarloDecision` is designed as a reporter class
since a reporter has access to both the sampler objects and the MultiApp system.

## Available adaptive Monte Carlo samplers

The following adaptive Monte Carlo samplers have been implemented in MOOSE:

- +Adaptive Importance Sampling (AIS)+

  AIS proposes and accepts the next input parameters using a Markov Chain Monte Carlo
  scheme to sample sufficiently in the failure region given a model. See [AdaptiveImportanceSampler](AdaptiveImportanceSampler.md)
  for more information.

- +Parallel Subset Simulation (PSS)+

  PSS can be used for characterizing probabilistic failure or performing optimization under uncertainty
  efficiently when dealing with expensive numerical models. This method works
  by creating intermediate failure thresholds and using hundreds of Markov chains to
  efficiently propagate to regions of the input parameter space that are most significant
  for either failure characterization or optimization. See [ParallelSubsetSimulation](ParallelSubsetSimulation.md) for more information.

!syntax parameters /Reporters/AdaptiveMonteCarloDecision

!syntax inputs /Reporters/AdaptiveMonteCarloDecision

!syntax children /Reporters/AdaptiveMonteCarloDecision
