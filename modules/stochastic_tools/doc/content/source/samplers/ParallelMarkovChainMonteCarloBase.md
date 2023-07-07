# ParallelMarkovChainMonteCarloBase (PMCMCBase)

!syntax description /Samplers/PMCMCBase

The `PMCMCBase` is the base class for performing MCMC sampling in STM and it derives from the [MonteCarloSampler](MonteCarloSampler.md) object. `PMCMCBase` overrides the `computeSample` function in the [MonteCarloSampler](MonteCarloSampler.md) which is the link to send new samples to the MultiApps system for execution. `PMCMCBase` takes as inputs the priors, number of parallel proposals, the experimental configurations, upper and lower bounds of the model parameters, and the initial values for the parameters representing the starting state of the Markov chain. The `PMCMCBase` object itself performs a regular Monte Carlo sampling, rather than a MCMC sampling. However, it provides functionalities to perform MCMC sampling through derived classes via several protected and public functions. An overview of these functions is presented in the Table below.

!table caption=Description of the functions in the `ParallelMarkovChainMonteCarloBase` class.
| Function | Type | Input | Output | Description
| - | - | - | - | - |
| `proposeSamples` | Protected | Random number seed | Void | Proposes new samples
| `sampleSetUp` | Protected | Void | Void | Sets up new samples and stores random numbers for later decision making
| `combineWithConfg` | Private | Void | Void | Combines the proposed samples with the experimental configurations
| `computeSample` | Protected | Row and column indices | Scalar real | Sends a sample of a parameter to the MultiApps system
| `getNumberofConfigValues` | Public | Void | Scalar real | Returns the number of experimental configurations
| `getNumberofConfigParams` | Public | Void | Scalar real | Returns the number of experimental configuration parameters
| `getNumberofParallelProposals` | Public | Void | Scalar real | Returns the number of parallel proposals
| `getRandomNumbers` | Public | Void | Vector real | Returns a vector of random number for decision making
| `getVarSamples` | Public | Void | Vector real | Returns a vector of samples of variances over the model predictions
| `getPriors` | Public | Void | Vector Distribution | Returns a vector of distribution objects of priors over the model parameters
| `getVarPrior` | Public | Void | Distribution | Returns a distribution object of prior over the variance term
| `decisionStep` | Public | Void | Integer | Get number of serial steps after which decision making starts

For classes deriving off of the `PMCMCBase`, it would suffice to override the `proposeSamples` function and fill in the protected `_new_samples` vector of vectors with new proposals. Then, the `PMCMCBase` will automatically combine these proposals with the experimental configurations and send the resulting samples to the `MultiApps` system. This treatment of combining the experimental configurations with the proposals only in the base class is very convenient. That is, if a new recipe needs to be implemented for combining experimental configurations and the proposals, this can only be done in the base class and this change will be reflected in all the derived classes. The specific re-definition of the `proposeSamples` function in the derived classes to propose samples can be found in per [IndependentGaussianMetropolisHastings](IndependentGaussianMetropolisHastings.md), [AffineInvariantStretchSampler](AffineInvariantStretchSampler.md), or [AffineInvariantDifferentialEvolutionSampler](AffineInvariantDifferentialEvolutionSampler.md).

!syntax parameters /Samplers/PMCMCBase

!syntax inputs /Samplers/PMCMCBase

!syntax children /Samplers/PMCMCBase
