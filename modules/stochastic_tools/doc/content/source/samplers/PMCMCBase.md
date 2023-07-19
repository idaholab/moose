# PMCMCBase (PMCMCBase)

!syntax description /Samplers/PMCMCBase

## Overview

The `PMCMCBase` is the base class for performing MCMC sampling in STM and it derives from the [MonteCarloSampler](MonteCarloSampler.md) object. `PMCMCBase` overrides the `computeSample` function in the [MonteCarloSampler](MonteCarloSampler.md) which is the link to send new samples to the MultiApps system for execution. `PMCMCBase` takes as inputs the priors, number of parallel proposals, the experimental configurations, upper and lower bounds of the model parameters, and the initial values for the parameters representing the starting state of the Markov chain. The `PMCMCBase` object itself performs a regular Monte Carlo sampling, rather than a MCMC sampling. However, it provides functionalities to perform MCMC sampling through derived classes via several protected and public functions. An overview of these functions is presented in the table below.

!table caption=Description of the functions in the `PMCMCBase` class.
| Function | Description
| - | - |
| `proposeSamples` | Proposes new samples
| `sampleSetUp` | Sets up new samples and stores random numbers for later decision making
| `combineWithExperimentalConfig` | Combines the proposed samples with the experimental configurations
| `computeSample` | Sends a sample of a parameter to the MultiApps system
| `getNumberofConfigValues` | Returns the number of experimental configurations
| `getNumberofConfigParams` | Returns the number of experimental configuration parameters
| `getNumberofParallelProposals` | Returns the number of parallel proposals
| `getRandomNumbers` | Returns a vector of random number for decision making
| `getVarSamples` | Returns a vector of samples of variances over the model predictions
| `getPriors` | Returns a vector of distribution objects of priors over the model parameters
| `getVarPrior` | Returns a distribution object of prior over the variance term
| `decisionStep` | Get number of serial steps after which decision making starts

For classes deriving off of the `PMCMCBase`, it would suffice to override the `proposeSamples` function and fill in the protected `_new_samples` vector of vectors with new proposals. Then, the `PMCMCBase` will automatically combine these proposals with the experimental configurations and send the resulting samples to the `MultiApps` system. This treatment of combining the experimental configurations with the proposals only in the base class is very convenient. That is, if a new recipe needs to be implemented for combining experimental configurations and the proposals, this can only be done in the base class and this change will be reflected in all the derived classes. The specific re-definition of the `proposeSamples` function in the derived classes to propose samples can be found in [IndependentGaussianMH](IndependentGaussianMH.md), [AffineInvariantStretchSampler](AffineInvariantStretchSampler.md), or [AffineInvariantDES](AffineInvariantDES.md).

!syntax parameters /Samplers/PMCMCBase

!syntax inputs /Samplers/PMCMCBase

!syntax children /Samplers/PMCMCBase
