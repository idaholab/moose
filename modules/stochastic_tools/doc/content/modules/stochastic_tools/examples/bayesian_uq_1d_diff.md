# Bayesian UQ on a 1D diffusion problem

This example demonstrates how to infer unknown model parameters given experimental observations. Uncertainties of the model parameters are inversely quantified through the Bayesian framework, which makes use of samplers including [IndependentGaussianMetropolisHastings.md], [AffineInvariantStretchSampler.md] or [AffineInvariantDifferentialEvolutionSampler.md].

## Overview of Bayesian UQ

In general, assume a computational model $y=f(\theta, x)$, where $\theta$ are unknown model parameters and $x$ are configuration parameters. Original belief about the model parameters, described by the prior distribution $p(\theta)$, is usually subject to large uncertainties.
Given experimental measurements $\mathcal{D}$ of the computational model at specific configuration conditions, updated knowledge about the model parameters $p(\theta|\mathcal{D})$ is given by the Bayes' theorem:

!equation
p(\theta|\mathcal{D}) \propto p(\mathcal{D}|\theta) p(\theta)

where $p(\mathcal{D}|\theta)$ is the likelihood function [Likelihoods](Likelihoods/index.md). For the [Gaussian](Gaussian.md) or [TruncatedGaussian.md] likelihood functions that are currently supported by the stochastic tools module, the standard deviation of the Gaussian/TruncatedGaussian distribution can be either fixed or inferred from the experimental measurements $\mathcal{D}$. The goal of Bayesian UQ is to infer the model parameters $\theta$ given experimental measurements $\mathcal{D}$, i.e., the posterior distributions $p(\theta|\mathcal{D})$.

The code architecture for performing MCMC sampling in a parallelized fashion is presented in Figure \ref{moose_mcmc}. There are three steps for performing the sampling: proposal, model evaluation, and decision making. These three steps and the respective code objects are discussed below:

1. `Proposals:` The proposals are made using the user-defined specific MCMC [Samplers](Samplers/index.md) object which derives from the MCMC base class. The specific MCMC sampler can be any variant of either serial or parallel MCMC techniques. In total, the MCMC sampler object proposes $P$ proposals which are subsequently sent to the [MultiApps](MultiApps/index.md) system.

2. `Model evaluation:` The [MultiApps](MultiApps/index.md) system executes the models for each of the parallel proposals. In total, there will be $P \times N$ model evaluations, where $N$ is the number of experimental configurations. [MultiApps](MultiApps/index.md) is responsible for distributing these jobs across a given set of processors.

3. `Decision making:` The model outputs from the [MultiApps](MultiApps/index.md) system are gathered by a [Reporters](Reporters/index.md) object to compute the likelihood function. Then, the acceptance probabilities are computed for the $P$ parallel proposals and `accept/reject' decisions are made. The information of accepted samples is sent to the MCMC sampler object to influence the next set of $P$ proposals.

The above three steps are repeated for a user-specified number of times.

!media bayesian_uq.png 
        style=display:block;margin-left:auto;margin-right:auto;width:40%;
        id=moose_mcmc 
        caption=Code architecture for parallelized MCMC sampling in MOOSE for performing Bayesian UQ ([!cite](fill_in_here)).


## Problem Statement

As the simpliest example, this tutorial demonstrates the application of Bayesian UQ on a 1D transient diffusion problem with Dirichlet boundary conditions on each end of the domain:

!equation
\frac{\partial u}{\partial t} + \frac{\partial^2 u}{\partial x^2} = 0 \quad x \in [0,L],

where $U(t, x)$ defines the solution field. The experiments measure the average solution field across the entire domain length $L$. Experimental measurements are pre-generated at eight configuration points $L=1.0, 2.0, ..., 8.0$, with a Gaussian noise with standard deviation 0.05 added to simulate the measurement error. The goal of the problem is to infer the lefthandside and righthandside Dirichilet boundary conditions, i.e., $U(t, x=0)$ and $U(t, x=L)$. It is worth pointing out that the $\sigma$ term, which is the variance of the sum of the experimental measurement and model deviations, can also be inferred during the Bayesian UQ process. For simplicity, this tutorial will first demonstrate the problem of inferring true boundary values by assuming a fixed variance term for the experimental error. Inference of the variance term will be explained in session [ToBeFilledIn]. 

## Inferring Calibration Parameters Only (Fixed $\sigma$ Term)

### Sub-Application Input

The problem defined above, with respect to the [MultiApps](MultiApps/index.md) system, is a sub-application. The complete input file for the problem is provided in [mcmc-sub]. The only item required
to enable the stochastic analysis is the [Controls](Controls/index.md) block, which contains a
[SamplerReceiver](/SamplerReceiver.md) object, the use of which will be explained in the following section.

!listing modules/stochastic_tools/test/tests/samplers/mcmc/sub.i
         id=mcmc-sub
         caption=Complete input file for executing the transient diffusion problem.


### Master Input

The master application, with respect to the [MultiApps](MultiApps/index.md) system, is the driver of the stochastic simulations. The master input does not perform a solve by itself, but has to be coupled with the sub-application for a full stochastic analysis. The complete input file for the master application using the base MCMC class is shown in [mcmc-base-master]. Details of each indivisual input section will is explained in details as follows.

1. The [StochasticTools](StochasticTools/index.md) block is defined at the beginning of the master application to automatically create the necessary objects for running a stochastic analysis. 

!listing modules/stochastic_tools/test/tests/samplers/mcmc/main_base.i block=StochasticTools

2. The [Distributions](Distributions/index.md) block is used to define the prior distributions for the two stochastic boundary conditions. In this example, the two stochastic boundary conditions are assumed to follow normal distributions 

!listing modules/stochastic_tools/test/tests/samplers/mcmc/main_base.i block=Distributions

3. A [Likelihoods](Likelihoods/index.md) object is created to specify the type of likelihood function used in the analysis. 

- A Gaussian-type log-likelihood function is used in the current analysis, as specified by `type = Gaussian` and `log_likelihood = true`. 
- The `noise` term defines the experimental noise plus model deviations against experiments. Variance of the Gaussian distribution is fixed to a constant value of 0.05 through the `noise_specified` term, as will be explained next in the [Reporters](Reporters/index.md) block. Note that this term can be inferred 
- The `file_name` argument takes in a CSV file that contains experimental values. In this example, the `exp_0.05.csv` file contains 8 independent experimental measurements, which are the sum of pre-simulated solution and a Gaussian noise with standard deviation of 0.05. 

!listing modules/stochastic_tools/test/tests/samplers/mcmc/main_base.i block=Likelihoods

4. In the [Samplers](Samplers/index.md) block, [PMCMCBase](ParallelMarkovChainMonteCarloBase.md) is used to sample the left and right boundary conditions according to the parallized Markov chain Monte Carlo method at certain configuration conditions. 

- `prior_distributions` takes in the pre-defined prior distributions of the parameters to be calibrated. 
- `num_parallel_proposals` specifies the number of proposals to make and corresponding sub-applications executed in parallel. 
- `initial_values` defines the starting values of the parameters to be calibrated. 
- `file_name` takes in a CSV file that contains the configuration values. In this example, the `confg.csv` file contains 8 configuration values $L=1.0, 2.0, ..., 8.0$. 

!listing modules/stochastic_tools/test/tests/samplers/mcmc/main_base.i block=Samplers

5. In the [MultiApps](MultiApps/index.md) block, a [SamplerFullSolveMultiApp](/SamplerFullSolveMultiApp.md) object is defined to create and run a sub-application for each sample provided by the sampler object. More specifically, a full-solve type sub-application is executed for each row of the Sampler matrix.

!listing modules/stochastic_tools/test/tests/samplers/mcmc/main_base.i block=MultiApps

6. Under the [Transfers](Transfers/index.md) block, [SamplerReporterTransfer](/SamplerReporterTransfer.md) is utilized to transfer data from [Reporters](Reporters/index.md) on the sub-aplication to a [StochasticReporter](/StochasticReporter.md) on the main application. In this example, the value of a postprocessor named `average`, as defined in the sub-application, is transferred to a [StochasticReporter](/StochasticReporter.md) named `constant` in the master application.

!listing modules/stochastic_tools/test/tests/samplers/mcmc/main_base.i block=Transfers

7. The [Controls](Controls/index.md) block makes use of [MultiAppSamplerControl](MultiAppSamplerControl.md) to pass command line arguments from the master application to the sub-applications. In this example, three arguments are passed from the master application to the sub-spplications, i.e.,

- `left_bc`: the Dirichlet boundary condition at the lefthandside of the domain. 
- `right_bc`: the Dirichlet boundary condition at the lefthandside of the domain.
- `mesh1`: the domain length, which is controled by the configuration parameters in the [Samplers](Samplers/index.md) block.

!listing modules/stochastic_tools/test/tests/samplers/mcmc/main_base.i block=Controls

8. Then, the [Reporters](Reporters/index.md) section defines `Reporter` objects for data transfer in the MultiApps system. 

- The `constant` reporter of type [StochasticReporter](/StochasticReporter.md) serves as a storage container for stochastic simulation results coming from the sub-application. 

- `noise_specified` makes use of the [ConstantReporter](/ConstantReporter.md) reporter to generate a constant value, which is used for the variance term in the experimental error in this example. The value is specified to be 0.05 as defined in `real_values`. 

- `mcmc_reporter` is of type [ParallelMarkovChainMonteCarloDecision](/ParallelMarkovChainMonteCarloDecision.md) which makes the decision whether or not to accept a proposal generated by the parallel Markov chain Monte Carlo algorithms. It takes in a likelihood function (`likelihoods`), a (`sampler`), and the model output from the sub-application (`output_value`). In this example, `output_value = constant/reporter_transfer:average:value` means that the model output from the sub-application is obtained by a reporter named `constant`, which interacts with the reporter `reporter_transfer` to transfer the average solution field.

!listing modules/stochastic_tools/test/tests/samplers/mcmc/main_base.i block=Reporters

!listing modules/stochastic_tools/test/tests/samplers/mcmc/main_base.i
    id=mcmc-base-master
    caption=Complete input file for master application for executing Bayesian UQ using the base MCMC class.

### Available Sampling Algorithms

Currently, multiple parallel MCMC samplers have been implemented in stochastic tools module, including the [ParallelMarkovChainMonteCarloBase](/ParallelMarkovChainMonteCarloBase.md), [IndependentGaussianMetropolisHastings](/IndependentGaussianMetropolisHastings.md), [AffineInvariantStretchSampler](/AffineInvariantStretchSampler.md), and [AffineInvariantDifferentialEvolutionSampler](/AffineInvariantDifferentialEvolutionSampler.md). The above example used [ParallelMarkovChainMonteCarloBase](/ParallelMarkovChainMonteCarloBase.md). Switching to a different sampling algorithm only involves minor changes in the [Samplers](Samplers/index.md) block and the [Reporters](Reporters/index.md) block in the master input file, while the input syntax for the sub-application remains the same. Details are explained below:

- [IndependentGaussianMetropolisHastings](IndependentGaussianMetropolisHastings.md): Performs Metropolis-Hastings MCMC sampling with independent Gaussian propoposals. Under the [Samplers](Samplers/index.md) block, `std_prop` specifies the standard deviations of the independent Gaussian proposals for making the next proposal. `seed_inputs` takes in a reporter named `mcmc_reporter` which reports seed inputs values for the next proposals. The `mcmc_reporter` is defined under the [Reporters](Reporters/index.md) block of type [IndependentMHDecision](Reporters/IndependentMetropolisHastingsDecision.md), which performs decision making for independent Metropolis-Hastings MCMC.

!listing modules/stochastic_tools/test/tests/samplers/mcmc/main_imh.i block=Samplers

!listing modules/stochastic_tools/test/tests/samplers/mcmc/main_imh.i block=Reporters

- [AffineInvariantStretchSampler](AffineInvariantStretchSampler.md): Performs affine-invariant ensemble MCMC with stretch sampler. Different from other samplers, it takes in two reporters to access the mean and variance of the previous state of all the walkers (`previous_state` and `previous_state_var`).  By default, the walkers take a step size of 0.2 in the stretch sampler. Correspondingly, an `mcmc_reporter` of type [AffineInvariantStretchDecision](Reporters/AffineInvariantStretchDecision.md) need be defined in the [Reporters](Reporters/index.md) block for decision making.

!listing modules/stochastic_tools/test/tests/samplers/mcmc/main_ss.i block=Samplers

!listing modules/stochastic_tools/test/tests/samplers/mcmc/main_ss.i block=Reporters

- [AffineInvariantDifferentialEvolutionSampler](AffineInvariantDifferentialEvolutionSampler.md): Performs affine-invariant ensemble MCMC with differential sampler. Similar to the stretch sampler, the evolution sampler requires two reporters to access the mean and variance of the previous state of all the walkers (`previous_state` and `previous_state_var`). Correspondingly, an `mcmc_reporter` of type [AffineInvariantDifferentialDecision](Reporters/AffineInvariantDifferentialDecision.md) need be defined in the [Reporters](Reporters/index.md) block for decision making.

!listing modules/stochastic_tools/test/tests/samplers/mcmc/main_des.i block=Samplers

!listing modules/stochastic_tools/test/tests/samplers/mcmc/main_des.i block=Reporters

## Inferring Both Calibration Parameters and the Variance Term

In addition to inferring the model parameters, the variance term can also be inferred at the same time. The differential evolution sampler [AffineInvariantDifferentialEvolutionSampler](AffineInvariantDifferentialEvolutionSampler.md) is needed for this purpose. Changes in the following blocks are necessary in the master input file.

- A prior distribution need be defined under the [Distributions](Distributions/index.md) block. In this example, a uniform distribution with wide uncertainty range is specified for the variance term as a conservative assumption.

!listing modules/stochastic_tools/test/tests/samplers/mcmc/main_des_var.i block=Distributions

- In the definition of the [Likelihoods](Likelihoods/index.md) block, the variance term for the Gaussian likelihood function is returned by a reporter named `mcmc_reporter`, which is defined in the [Reporters](Reporters/index.md) block of type [AffineInvariantDifferentialDecision](AffineInvariantDifferentialDecision.md). 

!listing modules/stochastic_tools/test/tests/samplers/mcmc/main_des_var.i block=Likelihoods

!listing modules/stochastic_tools/test/tests/samplers/mcmc/main_des_var.i block=Reporters

- Finally, in the [Samplers](Samplers/index.md) block, the [AffineInvariantDES](AffineInvariantDES.md) sampler takes in the prior distribution of the variance term via the `prior_variance` argument. In this example, the prior distribution of the variance term has been defined in `variance` under the [Distributions](Distributions/index.md) block.

!listing modules/stochastic_tools/test/tests/samplers/mcmc/main_des_var.i block=Samplers

The full example of using differential evolution sampler to infer both the model parameters and the variance term is shown in [infer-var].

!listing modules/stochastic_tools/test/tests/samplers/mcmc/main_des_var.i
    id=infer-var
    caption=Complete input file for inferring both model parameters and the variance term using the differential evolution sampler.

## Result and Analysis

The output `.json` file contains samples across different processors at different timesteps. Practically, all MCMC samples are generated with serial auto-correlations. Therefore, it is essential to diagnoze the sample auto-correlation for predictable UQ quality. Two primary diagnostics are presented here for analyzing the sample quality: the integrated auto-correlation time ($\tau_p$), and the effective sample size (ESS) [!cite](fill_in_later!!!!!!). $\tau_p$ describes the interval after which a sample can be considered independent given a current sample index, and ESS describes the number of "effective" samples after taking into consideration the sample auto-correlation. To obtain "independent" samples from the Markov chains, the first 100 samples are discarded for "burn-in", with the remaining samples thinned by $\tau_p/2$. The ESS of the generated samples are calculated based on the thinned samples.

As presented in [metrics], $\tau_p$ for IMH, SS and DES decreases, and ESS for the three algorithms increases corresponding, indicating the best sampling qualify by DES, followed by SS, then IMH. [samples] shows the posterior distributions of the calibration parameters $u_{left}$ and $u_{right}$. A strong linearatiy is observed for the two parameters as would be expected from the underlying physics. The IMH samples appear to be extremely localized compared to DES. The SS samples are almost as good as samples generated by DES in terms of exploring the parameter space. The DES samples are the best from this aspect. 



!table id=metrics
| Algorithm              | $\tau_p$                 | ESS                  |
|:-----------------------|:-------------------------|:---------------------|
| IMH                    | 123.91 (119.85, 127.96)  | 4.11 (3.96, 4.26)    |
| SS                     | 41.78  (42.84, 40.72)    | 265  (265, 265)      |
| DES                    | 11.22  (10.79, 11.64)    | 1500 (1500, 1500)    |


!media samples.png 
        style=display:block;margin-left:auto;margin-right:auto;width:40%;
        id=samples
        caption=Posterior distribution in Bayesian UQ.
