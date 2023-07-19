# PMCMCDecision (Parallel MCMC Decision)

!syntax description /Reporters/PMCMCDecision

## Overview

The `PMCMCDecision` is the base class for performing MCMC decision making in STM and it is a [Reporter](Reporters/index.md) object. `PMCMCDecision` overrides the `execute` function in the [Reporter](Reporters/index.md). `PMCMCDecision` takes as inputs the subApp outputs, the sampler object, and the likelihoods. The `PMCMCDecision` object itself facilitates a regular Monte Carlo sampling, rather than a MCMC sampling by accepting all the proposals made by the sampler class. However, it provides functionalities to perform MCMC sampling through derived classes via several protected and public functions. An overview of these functions is presented in the Table below.

!table caption=Description of the functions in the `PMCMCBase` class.
| Function | Description
| - |  - |
| `computeEvidence` |  Computes the logarithmic ratio of likelihood times prior of new to old samples
| `computeTransitionVector` | Computes the vector of acceptance probabilities
| `nextSamples` | Makes accept/reject decisions and transmits the accepted inputs and outputs
| `nextSeeds` | Initializes the next seed input for certain MCMC samplers
| `execute` | Main code block which facilitates evidence and transition probability computations, accept/reject decisions, and transmission of results to JSON. Ideally, this should not be overriden by derived classes. Any changes to this in the base will effect all MCMC decision classes behavior

The `execute` function in `PMCMCDecision` is typically not meant to be overridden by the derived classes, although this can be done. `execute` gathers the MultiApps outputs from all the processors, sets up a matrix of outputs and input parameters, computes the evidence and transition vectors, assembles the accepted samples based on the transition probabilities, and transmits the accepted sample inputs and outputs to a JSON file. For some MCMC samplers like the [IndependentMHDecision](IndependentMHDecision.md) which relies on a single seed to propose the next samples, the `execute` function also calls the `nextSeeds` function which is overridable. Finally, the construction of the `execute` is assumed to uniformly apply to all MCMC samplers and it should be noted that any changes made to it can influence the behavior of existing and future MCMC samplers.

A decision making class for a new MCMC sampler should derive from `PMCMCDecision` and typically only override the `computeTransitionVector` function. The [IndependentMHDecision](IndependentMHDecision.md) sampler is an exception because it relies on a single seed.

!syntax parameters /Reporters/PMCMCDecision

!syntax inputs /Reporters/PMCMCDecision

!syntax children /Reporters/PMCMCDecision
