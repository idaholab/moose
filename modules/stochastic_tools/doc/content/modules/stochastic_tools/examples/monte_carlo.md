# Monte Carlo Example

Possibly the simplest example of a stochastic analysis is to perform Monte Carlo analysis of a given
simulation. That is, solve a single problem and vary parameters within this simulation with a random
set of perturbed parameters.

## Problem Statement

The first step is to define the simulation to perform, in this case the simulation considered is a 1D
transient diffusion equation with Dirichlet boundary conditions on each end of the domain: find $u$
such that

!equation
\frac{\partial u}{\partial t} + \frac{\partial^2 u}{\partial x^2} = 0 \quad x \in [0,1],

where $u(0) = U(0, 0.5)$, $u(1) = U(1,2)$, and $U(a,b)$ defines a
[continuous uniform distribution](https://en.wikipedia.org/wiki/Uniform_distribution_%28continuous%29)
with $a$ and $b$ defining the lower and upper limits of the distribution, respectively.

## Sub-Application Input

The problem defined above, with respect to the [MultiApps] system, is a sub-application. The
complete input file for the problem is provided in [monte-carlo-sub]. The only item required
to enable the stochastic analysis is the [Controls] block, which contains a
[SamplerReceiver](/SamplerReceiver.md) object, the use of which will be explained
in the following section.

!listing modules/stochastic_tools/test/tests/transfers/monte_carlo/sub.i
         id=monte-carlo-sub
         caption=Complete input file for executing the transient diffusion problem.

## Master Input

The master application, with respect to the [MultiApps] system, is the driver of the stochastic
simulations, by itself it does not perform a solve. The complete input file for the master
application is shown in [monte-carlo-master], but the import sections will be detailed individually.

First, [Distributions] for each of the two stochastic boundary conditions are defined.

!listing modules/stochastic_tools/test/tests/transfers/monte_carlo/monte_carlo.i block=Distributions

Second, a [MonteCarloSampler](/MonteCarloSampler.md) is defined that utilizes the
two distributions and creates the Monte Carlo samples.

!listing modules/stochastic_tools/test/tests/transfers/monte_carlo/monte_carlo.i block=Samplers

Notice, that this sampler only executes on "initial", which means that the random numbers are
created once during the initial setup of the problem and not changed again during the simulation.

Next, a [SamplerTransientMultiApp](/SamplerTransientMultiApp.md) object is created. This object
creates and runs a sub-application for each sample provided by the sampler object.

!listing modules/stochastic_tools/test/tests/transfers/monte_carlo/monte_carlo.i block=MultiApps

Finally, the [SamplerParameterTransfer](/SamplerParameterTransfer.md) is utilized to communicate the
sampler data to the sub-application. The 'parameters' input lists the parameters on the
sub-applications to perturb.

!listing modules/stochastic_tools/test/tests/transfers/monte_carlo/monte_carlo.i block=Transfers

!listing modules/stochastic_tools/test/tests/transfers/monte_carlo/monte_carlo.i
         id=monte-carlo-master
         caption=Complete input file for master application for executing Monte Carlo stochastic
                 simulations.
