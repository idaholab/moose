# Bayesian Ucertainty Quantification (UQ) on a 1D Diffusion Problem

This example demonstrates how to infer unknown model parameters given experimental observations. Uncertainties of the model parameters are inversely quantified through the Bayesian framework, which makes use of samplers including [IndependentGaussianMH.md], [AffineInvariantStretchSampler.md] or [AffineInvariantDES.md].

## Overview of Bayesian UQ

In general, assume a computational model $\mathcal{M}(\theta)$, where $\theta$ are unknown model parameters. Original belief about the unknown parameters, described by the prior distribution $f(\pmb{\theta}, \sigma)$, is usually subject to large uncertainties.
Given experimental measurements $\pmb{\mathcal{D}}$ of the computational model at specific configuration conditions $\pmb{\Theta}$, updated knowledge about the model parameters $f(\pmb{\theta}, \sigma | \pmb{\Theta}, \mathcal{M}, \pmb{\mathcal{D}})$ can be obtained by the Bayes' theorem:

!equation
f(\pmb{\theta}, \sigma | \pmb{\Theta}, \mathcal{M}, \pmb{\mathcal{D}}) \propto \mathcal{L}(\pmb{\theta}, \sigma | \pmb{\Theta}, \mathcal{M}, \pmb{\mathcal{D}})~f(\pmb{\theta}, \sigma)

where $\mathcal{L}(\pmb{\theta}, \sigma | \pmb{\Theta}, \mathcal{M}, \pmb{\mathcal{D}})$ is the [Likelihood](Likelihood/index.md) function. [Gaussian](Gaussian.md) and [TruncatedGaussian.md] distributions are the most popular choices for the likelihood function. Variance of the Guassian or truncated Gaussian, which describes the sum of the model discrepancy and measurement error (referred to as $\sigma$ term herein), can be either fixed apriori or inferred from the experimental measurements $\pmb{\mathcal{D}}$. The goal of Bayesian UQ is to infer the unknown parameters $\pmb{\theta}$ (and $\sigma$ if needed) given experimental measurements $\pmb{\mathcal{D}}$, i.e., the posterior distributions $f(\pmb{\theta}, \sigma | \pmb{\Theta}, \mathcal{M}, \pmb{\mathcal{D}})$.

The code architecture for performing Markov Chain Monte Carlo (MCMC) sampling in a parallelized fashion is presented in [moose_mcmc]. There are three steps for performing the sampling: proposal, model evaluation, and decision making. These three steps and the respective code objects are discussed below:

1. `Proposals:` The proposals are made using the user-defined specific MCMC [Samplers](Samplers/index.md) object which derives from the MCMC base class [PMCMCBase](PMCMCBase.md). The specific MCMC sampler can be any variant of either serial or parallel MCMC techniques. In total, the MCMC sampler object proposes $P$ proposals which are subsequently sent to the [MultiApps](MultiApps/index.md) system.

2. `Model evaluation:` The [MultiApps](MultiApps/index.md) system executes the models for each of the parallel proposals. In total, there will be $P \times N$ model evaluations, where $N$ is the number of experimental configurations. [MultiApps](MultiApps/index.md) is responsible for distributing these jobs across a given set of processors.

3. `Decision making:` The model outputs from the [MultiApps](MultiApps/index.md) system are gathered by a [Reporters](Reporters/index.md) object to compute the likelihood function. Then, the acceptance probabilities are computed for the $P$ parallel proposals and accept/reject decisions are made. The information of accepted samples is sent to the MCMC sampler object to influence the next set of $P$ proposals.

The above three steps are repeated for a user-specified number of times.

!media bayesian_uq.png 
        style=display:block;margin-left:auto;margin-right:auto;width:80%;
        id=moose_mcmc 
        caption=Code architecture for parallelized MCMC sampling in MOOSE for performing Bayesian UQ ([!cite](dhulipala2023massively)).


## Problem Statement

This tutorial demonstrates the application of Bayesian UQ on a 1D transient diffusion problem with Dirichlet boundary conditions on each end of the domain:

!equation
\frac{\partial u}{\partial t} - \frac{\partial^2 u}{\partial x^2} = 0 \quad x \in [0,L],

!equation
u(t=0, x) = 0

!equation
\begin{aligned}
        u(t, x=0) = u_{\mathrm{left}} \\
        u(t, x=L) = u_{\mathrm{right}}
\end{aligned}

where $u(t, x)$ defines the solution field. The experiments measure the average solution field across the entire domain length $L$ at the end of time $T$. Experimental measurements are pre-generated at eight configuration points $L=1.0, 2.0, ..., 8.0$. A Gaussian noise with standard deviation of 0.05 is added to simulate the measurement error. The goal is to infer the lefthandside and righthandside Dirichilet boundary conditions, i.e., $u(x=0)$ and $u(x=L)$, referred to as $u_{\mathrm{left}}$ and $u_{\mathrm{right}}$ for conciseness. It is worth pointing out that the $\sigma$ term, which is the variance of the sum of the experimental measurement and model deviations, can also be inferred during the Bayesian UQ process. This tutorial will first demonstrate the inference of true boundary values by assuming a fixed variance term for the experimental error, followed by the inference of the boundary values together with variance term. 

## Inferring Calibration Parameters Only (Fixed $\sigma$ Term)

### Sub-Application Input

The problem defined above, with respect to the [MultiApps](MultiApps/index.md) system, is a sub-application. The complete input file for the problem is provided in [mcmc-sub].

!listing modules/stochastic_tools/test/tests/samplers/mcmc/sub.i
         id=mcmc-sub
         caption=Complete input file for executing the transient diffusion problem.


### Main Input

The main application, with respect to the [MultiApps](MultiApps/index.md) system, is the driver of the stochastic simulations. The main input does not perform a solve itself, couples with the sub-application for the full stochastic analysis. Details of each individual input section will is explained in details as follows.

1. The [StochasticTools](StochasticTools/index.md) block is defined at the beginning of the main application to automatically create the necessary objects for running a stochastic analysis. 

!listing modules/stochastic_tools/test/tests/samplers/mcmc/main_des.i block=StochasticTools

2. The [Distributions](Distributions/index.md) block is used to define the prior distributions for the two stochastic boundary conditions. In this example, the two stochastic boundary conditions are assumed to follow normal distributions 

!listing modules/stochastic_tools/test/tests/samplers/mcmc/main_des.i block=Distributions

3. A [Likelihoods](Likelihood/index.md) object is created to specify the type of likelihood function used in the analysis. 

- A Gaussian-type log-likelihood function is used in the current analysis, as specified by `type = Gaussian` and `log_likelihood = true`. 
- The `noise` term defines the experimental noise plus model deviations against experiments. Scale of the Gaussian distribution is fixed to a constant value of 0.05 through the `noise_specified` term, as will be explained next in the [Reporters](Reporters/index.md) block. Note that this term can be inferred 
- The `file_name` argument takes in a CSV file that contains experimental values. In this example, the `exp_0.05.csv` file contains 8 independent experimental measurements, which are the sum of pre-simulated solution and a Gaussian noise with standard deviation of 0.05. 

!listing modules/stochastic_tools/test/tests/samplers/mcmc/main_des.i block=Likelihood

4. In the [Samplers](Samplers/index.md) block, [AffineInvariantDES](/AffineInvariantDES.md) is used to sample the left and right boundary conditions according to the parallelized Markov chain Monte Carlo method at certain configuration conditions. 

- `prior_distributions` takes in the pre-defined prior distributions of the parameters to be calibrated. 
- `num_parallel_proposals` specifies the number of proposals to make and corresponding sub-applications executed in parallel. 
- `initial_values` defines the starting values of the parameters to be calibrated. 
- `file_name` takes in a CSV file that contains the configuration values. In this example, the `confg.csv` file contains 8 configuration values $L=1.0, 2.0, ..., 8.0$. 

!listing modules/stochastic_tools/test/tests/samplers/mcmc/main_des.i block=Samplers

5. In the [MultiApps](MultiApps/index.md) block, a [SamplerFullSolveMultiApp](/SamplerFullSolveMultiApp.md) object is defined to create and run a sub-application for each sample provided by the sampler object. More specifically, a full-solve type sub-application is executed for each row of the Sampler matrix.

!listing modules/stochastic_tools/test/tests/samplers/mcmc/main_des.i block=MultiApps

6. Under the [Transfers](Transfers/index.md) block, [SamplerReporterTransfer](/SamplerReporterTransfer.md) is utilized to transfer data from [Reporters](Reporters/index.md) on the sub-aplication to a [StochasticReporter](/StochasticReporter.md) on the main application. In this example, the value of a postprocessor named `average`, as defined in the sub-application, is transferred to a [StochasticReporter](/StochasticReporter.md) named `constant` in the main application.

!listing modules/stochastic_tools/test/tests/samplers/mcmc/main_des.i block=Transfers

7. The [Controls](Controls/index.md) block makes use of [MultiAppSamplerControl](MultiAppSamplerControl.md) to pass command line arguments from the main application to the sub-applications. In this example, three arguments are passed from the main application to the sub-spplications, i.e.,

- `left_bc`: the Dirichlet boundary condition at the lefthandside of the domain. 
- `right_bc`: the Dirichlet boundary condition at the lefthandside of the domain.
- `mesh1`: the domain length, which is controled by the configuration parameters in the [Samplers](Samplers/index.md) block.

!listing modules/stochastic_tools/test/tests/samplers/mcmc/main_des.i block=Controls

8. Then, the [Reporters](Reporters/index.md) section defines `Reporter` objects for data transfer in the MultiApps system. 

- The `constant` reporter of type [StochasticReporter](/StochasticReporter.md) serves as a storage container for stochastic simulation results coming from the sub-application. 

- `noise_specified` makes use of the [ConstantReporter](/ConstantReporter.md) reporter to generate a constant value, which is used for the variance term in the experimental error in this example. The value is specified to be 0.05 as defined in `real_values`. 

- `mcmc_reporter` is of type [AffineInvariantDES](/AffineInvariantDES.md) which makes the decision whether or not to accept a proposal generated by the parallel Markov chain Monte Carlo algorithms. It takes in a likelihood function `likelihoods`, a `sampler`, and the model output from the sub-application `output_value`. In this example, `output_value = constant/reporter_transfer:average:value` means that the model output from the sub-application is obtained by a reporter named `constant`, which interacts with the reporter `reporter_transfer` to transfer the average solution field.

!listing modules/stochastic_tools/test/tests/samplers/mcmc/main_des.i block=Reporters


### Available Sampling Algorithms

Currently, multiple parallel MCMC samplers have been implemented in stochastic tools module, including [IndependentGaussianMH](/IndependentGaussianMH.md), [AffineInvariantStretchSampler](/AffineInvariantStretchSampler.md), and [AffineInvariantDES](/AffineInvariantDES.md). The above example is demonstrated on [AffineInvariantDES](/AffineInvariantDES.md). Switching to a different MCMC sampler only requires minor changes in the [Samplers](Samplers/index.md) block and the [Reporters](Reporters/index.md) block in the main input file, while the input syntax for the sub-application remains the same. Three available MCMC samplers are explained below respectively:

- [IndependentGaussianMH](IndependentGaussianMH.md)(IMH): Performs Metropolis-Hastings MCMC sampling with independent Gaussian propoposals. Under the [Samplers](Samplers/index.md) block, `std_prop` specifies the standard deviations of the independent Gaussian proposals for making the next proposal. `seed_inputs` takes in a reporter named `mcmc_reporter` which reports seed inputs values for the next proposals. The `mcmc_reporter` is defined under the [Reporters](Reporters/index.md) block of type [IndependentMHDecision](reporters/IndependentMHDecision.md), which performs decision making for independent Metropolis-Hastings MCMC [!citep](calderhead2014general).

!listing modules/stochastic_tools/test/tests/samplers/mcmc/main_imh.i block=Samplers

!listing modules/stochastic_tools/test/tests/samplers/mcmc/main_imh.i block=Reporters

- [AffineInvariantStretchSampler](AffineInvariantStretchSampler.md)(SS): Performs affine-invariant ensemble MCMC with stretch sampler [!citep](goodman2010ensemble, shields2021subset). Different from other samplers, it takes in two reporters to access the mean and variance of the previous state of all the walkers (`previous_state`).  By default, the walkers take a step size of 0.2 in the stretch sampler. Correspondingly, an `mcmc_reporter` of type [AffineInvariantStretchDecision](reporters/AffineInvariantStretchDecision.md) need be defined in the [Reporters](Reporters/index.md) block for decision making.

!listing modules/stochastic_tools/test/tests/samplers/mcmc/main_ss.i block=Samplers

!listing modules/stochastic_tools/test/tests/samplers/mcmc/main_ss.i block=Reporters

- [AffineInvariantDES](AffineInvariantDES.md)(DES): Performs affine-invariant ensemble MCMC with differential sampler [!citep](braak2006markov, nelson2013rundmc). Similar to the stretch sampler, the evolution sampler requires two reporters to access the mean and variance of the previous state of all the walkers (`previous_state`). Correspondingly, an `mcmc_reporter` of type [AffineInvariantDifferentialDecision](reporters/AffineInvariantDifferentialDecision.md) need be defined in the [Reporters](Reporters/index.md) block for decision making.

!listing modules/stochastic_tools/test/tests/samplers/mcmc/main_des.i block=Samplers

!listing modules/stochastic_tools/test/tests/samplers/mcmc/main_des.i block=Reporters

## Inferring Both Calibration Parameters and the Variance Term

In addition to inferring the model parameters, the variance term can also be inferred at the same time. Both the differential evolution sampler [AffineInvariantDES](AffineInvariantDES.md) or stretch sampler [AffineInvariantStretchSampler](AffineInvariantStretchSampler.md) can be used for this purpose. Changes in the following blocks are necessary in the main input file.

- A prior distribution need be defined under the [Distributions](Distributions/index.md) block. In this example, a uniform distribution with wide uncertainty range is specified for the variance term as a conservative assumption.

!listing modules/stochastic_tools/test/tests/samplers/mcmc/main_des_var.i block=Distributions

- In the definition of the [Likelihood](Likelihood/index.md) block, the variance term for the Gaussian likelihood function is returned by a reporter named `mcmc_reporter`, which is defined in the [Reporters](Reporters/index.md) block of type [AffineInvariantDifferentialDecision](AffineInvariantDifferentialDecision.md). 

!listing modules/stochastic_tools/test/tests/samplers/mcmc/main_des_var.i block=Likelihood

!listing modules/stochastic_tools/test/tests/samplers/mcmc/main_des_var.i block=Reporters

- Finally, in the [Samplers](Samplers/index.md) block, the [AffineInvariantDES](AffineInvariantDES.md) sampler takes in the prior distribution of the variance term via the `prior_variance` argument. In this example, the prior distribution of the variance term has been defined in `variance` under the [Distributions](Distributions/index.md) block.

!listing modules/stochastic_tools/test/tests/samplers/mcmc/main_des_var.i block=Samplers


## Result and Analysis

The output JSON file contains samples across different processors at different timesteps. Practically, all MCMC samples are generated with serial auto-correlations. Therefore, it is essential to diagnoze the sample auto-correlation for predictable UQ quality. Two primary diagnostics are presented here for analyzing the sample quality: the integrated auto-correlation time ($\tau_p$), and the effective sample size (ESS) [!cite](dhulipala2023massively). $\tau_p$ describes the interval after which a sample can be considered independent given a current sample index, and ESS describes the number of "effective" samples after taking into consideration the sample auto-correlation. To obtain "independent" samples from the Markov chains, the first 100 samples are discarded for "burn-in", with the remaining samples thinned by $\tau_p/2$. The ESS of the generated samples are calculated based on the thinned samples. As presented in [metrics_case1], $\tau_p$ for IMH, SS and DES decreases, and ESS for the three algorithms increases correspondingly, indicating the best sampling qualify by DES, followed by SS, then IMH. 

!table id=metrics_case1
        caption=Integrated autocorrelation time ($\tau_p$) and effective sample size (ESS) when inferring model parameters
| Algorithm              | $\tau_p$                 | ESS                  |
|:-----------------------|:-------------------------|:---------------------|
| IMH                    | 123.908                  | 9.528                |
| SS                     | 41.779                   | 46.719               |
| DES                    | 11.217                   | 676.547              |

When inferring only the boundary conditions, all three samplers are applied, with 1,000 serial steps simulated with 5 parallel proposals per step.
[scatter_plots_case1] shows the posterior distributions of the calibration parameters $u_{\mathrm{left}}$ and $u_{\mathrm{right}}$. A strong linearity is observed for the two parameters as would be expected from the underlying physics. The IMH samples appear to be extremely localized compared to DES. The SS samples are almost as good as samples generated by DES in terms of exploring the parameter space. The DES samples are the best from this aspect. More detailed posterior distributions are presented in [case1_inference_results] for all three algorithms IMH, SS and DES. It is generally observed that the posteriors using IMH are restricted in the parameter space. Whereas, the posteriors for both SS and DES are spread across the parameter space and look comparable to each other. This can be observed by comparing [case1_inference_results]b and [case1_inference_results]c for the posterior of $u_{\mathrm{left}}$, and [case1_inference_results]e and [case1_inference_results]f for the posterior of $u_{\mathrm{right}}$. 

!media dtd_inf1_params.png
        style=display:block;margin-left:auto;margin-right:auto;width:50%;
        id=scatter_plots_case1
        caption=Scatter plot of the samples from IMH, SS, and DES samplers when only inferring the model parameters.

!media case1_inference_results.png
        style=display:block;margin-left:auto;margin-right:auto;width:80%;
        id=case1_inference_results
        caption=Posterior distributions for the diffusion time derivative problem when inferring only the model parameters. (a), (b) and (c) are the $u_{\mathrm{left}}$ posteriors using IMH, SS, and DES samplers, respectively. (d), (e) and (f) are the $u_{\mathrm{right}}$ posteriors using IMH, SS, and DES samplers, respectively.

When inferring both boundary conditions and the $\sigma$ term, only the SS and DES samplers are considerend given the poor performance of the IMH sampler evidenced from the above case. Again, 1,000 serial steps are simulated with 5 parallel proposals per step. Sampling diagnostics are presented for the two samplers in [metrics_case2]. The $\tau_p$ for both samplers are comparable, and the $\tau_p$ for DES increased compared to the previous case of inferring only model parameters, possible caused by the increased problem complexity due to the inclusion of the $\sigma$ term. For the ESS, the DES shows a three time enhancement compared to SS, due to the higher residual sample autocorrelation upon chain thinning.

!table id=metrics_case2
        caption=Integrated autocorrelation time ($\tau_p$) and effective sample size (ESS) when inferring both model parameters and the $\sigma$ term.
| Algorithm              | $\tau_p$                 | ESS                  |
|:-----------------------|:-------------------------|:---------------------|
| SS                     | 36.228                   | 51.991               |
| DES                    | 45.542                   | 155.0                |

Scatter plot of the inferred model parameters $u_{\mathrm{left}}$ and $u_{\mathrm{right}}$ is presented in [scatter_plots_case2]. Again, the two model parameters exhibit almost a perfect correlation which is consistent with the observation in the previous case, due to the ill-posedness of the problem.

!media bayesian_uq/dtd_inf2_params.png
        style=display:block;margin-left:auto;margin-right:auto;width:50%;
        id=scatter_plots_case2
        caption=Scatter plot of the samples from IMH, SS, and DES samplers when inferring both model parameters and the $\sigma$ term.

Detailed posterior distributions of $u_{\mathrm{left}}$, $u_{\mathrm{right}}$ and the $\sigma$ term are presented in [case2_inference_results] for the SS and DES samplers. Both samplers are producing quite consistent posterior distributions. Specifically, [case2_inference_results]a and [case2_inference_results]d need to be compared for $u_{\mathrm{left}}$, [case2_inference_results]b and [case2_inference_results]e for $u_{\mathrm{right}}$, and [case2_inference_results]c and [case2_inference_results]f for the $\sigma$ term. Both samplers produce consistent posterior distributions, and the posterior
distributions of the $\sigma$ term are approximately centered around the true value of the Gaussian noise (denoted by the solid vertical line) that was added to replicate the “experimental” data. 

!media case2_inference_results.png
        style=display:block;margin-left:auto;margin-right:auto;width:80%;
        id=case2_inference_results
        caption=Posterior distributions for the diffusion time derivative problem when inferring the model parameters and the $\sigma$ term. (a), (b) and (c) show the posteriors using SS. (d), (e) and (f) show the posteriors using DES.

As shown in [case1_inference_results] and [case2_inference_results], posteriors of the model parameters and potentially the $\sigma$ term are much more constrained compared to the priors, reflecting the integration of experimental information into the models through the Bayesian UQ process. Followup forward UQ based on the updated posteriors will provide more accurate information about the output uncertainty.