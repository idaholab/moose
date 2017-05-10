# MonteCarloSampler

Monte Carlo Sampler is a computational algorithm based on random sampling a probability density function (PDF)
to obtain numerical results. The general procedure is:

1. Define a domain of possible input paramters;
2. Generate input paramters randomly from a prescribed PDF over the domain;
3. Perform a model calculations with the sampled input parameters;
4. Aggregate the outputs.

## Example Input Syntax
!listing modules/stochastic_tools/tests/samplers/monte_carlo/sampler_materials_test.i block=Samplers

!parameters /Samplers/MonteCarloSampler

!inputfiles /Samplers/MonteCarloSampler

!childobjects /Samplers/MonteCarloSampler
