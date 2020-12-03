# Metropolis

!syntax description /Samplers/Metropolis

## Overview

This sampler implements the Metropolis Markov Chain Monte Carlo (MCMC) method
presented in [!cite](au2014engineering). The Metropolis MCMC algorithm proceeds as follows:

- Initialize the sampler with an initial value and a standard deviation for the proposal
- Propose a new value from a symmetric distribution such as the Gaussian distribution
- Evaluate the acceptance ratio using the equation below
- If acceptance ratio is greater than a uniform random number between 0 and 1, accept
  the proposal as the new sample $i$ with probability \alpha
- Otherwise the new sample is the previous sample $i-1

!equation id=eqn:metropolis
\alpha = \frac{p(x^*)}{p(x^{i-1})}

where, p(.) denotes the probability distribution of the parameter. The Metropolis
algorithm is repeated until enough number of samples are generated. Multiple Markov
chains can also be run in parallel.

## Example Input Syntax

The following file creates the Metropolis sampler for two input parameters having Weibull and
Uniform distributions. [!param](/Samplers/Metropolis/num_rows) denotes the number of
Markov chains. [!param](/Samplers/Metropolis/inputs_vpp) provides Metropolis sampler with the previous sample in
order to decide the next sample. [!param](/Samplers/Metropolis/proposal_std) contains the standard deviations
 of the proposal distributions for the two parameters. [!param](/Samplers/Metropolis/initial_values) are the
 starting values for the Metropolis sampler.

!listing modules/stochastic_tools/test/tests/samplers/metropolis/metropolis.i block=Samplers

It should be noted that this Metropolis implements a Gaussian proposal distribution.
In addition to the `Sampler` block, there should also be an `Executioner` block, where,
 [!param](/Executioner/Transient/num_steps) controls the number of MCMC samples per Markov Chain.

!listing modules/stochastic_tools/test/tests/samplers/metropolis/metropolis.i block=Executioner

!syntax parameters /Samplers/Metropolis

!syntax inputs /Samplers/Metropolis

!syntax children /Samplers/Metropolis
