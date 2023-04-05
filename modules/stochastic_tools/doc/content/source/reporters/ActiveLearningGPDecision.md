# ActiveLearningGPDecision

!syntax description /Reporters/ActiveLearningGPDecision

## Description

The `ActiveLearningGPDecision` is a Reporter class that performs the following functions to facilitate the active learning process in a parallelized fashion:

- For every new batch of input samples produced by the [ActiveLearningMonteCarloSampler](ActiveLearningMonteCarloSampler.md), the corresponding Gaussian Process (GP) predictions are made.
- These GP predictions are assessed for their quality by using the uncertainty information. If any of the predictions are of poor quality in the batch, those samples are marked and this information is sent back to the [ActiveLearningMonteCarloSampler](ActiveLearningMonteCarloSampler.md).
- Once a user-specified batch size of inputs for running the high-fidelity model evaluations (i.e., those input samples for which the GP predictions are poor) is met and these evaluations are executed in parallel, re-train the GP model via the [ActiveLearningGaussianProcess](ActiveLearningGaussianProcess.md) Surrogate class.
- Output the GP mean predictions and standard deviations and corresponding inputs to a json file. Also indicate those input samples for which a high-fidelity model evaluation was necessary during the active learning.

## Why Gaussian Process surrogate for active learning?

A GP is a Bayesian surrogate model. This means, in addition to making predictions of the high-fidelity model outputs, a GP produces uncertainty estimates of its outputs. Higher the uncertainty, lesser the confidence in the GP predictions. An illustration of the GP predictions and associated uncertainties is presented in [!ref](al1_sch). In addition, many studies (e.g., [!cite](xiu2019al) and [!cite](dhulipala2022al)) have found that the uncertainty estimates produced by a GP are robust. This in-built uncertainty information permits us to create an active feedback loop between the training data and GP in that we can use the GP uncertainty estimates to select the next best training data point(s). As such, we can even start the active learning process using a GP that is trained with little data.

!media GP_optimization.svg style=width:50%; id=al1_sch caption=Schematic of active learning in Monte Carlo simulations with parallel computing. The interaction between the three objects, [ActiveLearningMonteCarloSampler](ActiveLearningMonteCarloSampler.md), [ActiveLearningGaussianProcess](ActiveLearningGaussianProcess.md), and `ActiveLearningGPDecision`, is presented.

Of the spectrum of Bayesian surrogate models, a GP is the most straight-forward to train. First, during active learning, the training data set size would typically be small (in the order of hundreds) for most problems. Therefore, re-training the GP should be associated with a low computational cost. Even in those extreme cases where the training data set size becomes large, we can adopt stochastic optimization methods like Adam optimization [!cite](kingma2014adam) (this feature is available in MOOSE [GaussianProcessTrainer](GaussianProcessTrainer.md)) to select a mini-batch of the training data set and substatially reduce the computational expense.

Owing to these reasons, a GP surrogate is usually preferred for active learning. However, for the sake of completeness, we note that there have been other studies that have used different surrogates like a polynomial chaos expansion for active learning (e.g., [!cite](marelli2019al)).

## Procedure for active learning in MOOSE

In general, active learning has two phases: (1) initial training phase for the GP; and (2) the active learning phase.

[!ref](al2_sch) presents the initial training phase of the GP surrogate. The [ActiveLearningMonteCarloSampler](ActiveLearningMonteCarloSampler.md) sends a batch of input samples for high-fidelity model executions to the [SamplerFullSolveMultiApp](SamplerFullSolveMultiApp.md). These high-fidelity models executions are performed in parallel using separate sets of processors. Outputs from the model executions will be collected by the `ActiveLearningGPDecision` reporter. Once a required number of training data size is reached, `ActiveLearningGPDecision` will call the [ActiveLearningGaussianProcess](ActiveLearningGaussianProcess.md) to initially train the GP model.

!media ActiveLearning_training.png style=width:75%; id=al2_sch caption=Schematic of the initial training phase of the GP surrogate.

[!ref](al3_sch) presents the subsequent active learning phase. The [ActiveLearningMonteCarloSampler](ActiveLearningMonteCarloSampler.md) sends a batch of input samples to the `ActiveLearningGPDecision` reporter. Then, the [ActiveLearningGaussianProcess](ActiveLearningGaussianProcess.md) is called to make GP predictions. In addition, `ActiveLearningGPDecision` assesses the quality of the GP predictions through user-defined acquistion (or learning) functions. Some popular acquisition functions are currently available, and it is easy to include additional functions in the `ActiveLearningGPDecision` class. If one or more of the GP predictions are poor (as indicated by the acquistion function), the corresponding input samples are marked and this information is communicated to the [ActiveLearningMonteCarloSampler](ActiveLearningMonteCarloSampler.md). This process continues until a user-defined batch size is met for the maximum number of poor quality GP predictions. Once this threshold is reached, the [ActiveLearningMonteCarloSampler](ActiveLearningMonteCarloSampler.md) sends the batch of those book-marked input samples whose GP predictions are poor for high-fidelity model evaluations. The batch of high-fidelity models can be run in parallel. Then, the high-fidelity model outputs are acquired by the `ActiveLearningGPDecision` reporter where the GP model is re-trained. This process then repeats until the user-specified number of Monte Carlo samples is met.

!media ActiveLearning_consolidated.svg style=width:75%; id=al3_sch caption=Schematic of active learning in Monte Carlo simulations with parallel computing. The interaction between the three objects, [ActiveLearningMonteCarloSampler](ActiveLearningMonteCarloSampler.md), [ActiveLearningGaussianProcess](ActiveLearningGaussianProcess.md), and `ActiveLearningGPDecision`, during the active learning phase.

## Input file syntax

The full input file driver for active learning is presented below. The three main components of the input file are [ActiveLearningMonteCarloSampler](ActiveLearningMonteCarloSampler.md), [ActiveLearningGaussianProcess](ActiveLearningGaussianProcess.md), and `ActiveLearningGPDecision`. These are discussed in detail below.

!listing modules/stochastic_tools/test/tests/reporters/ActiveLearningGP/main_adam.i

### `ActiveLearningMonteCarloSampler`

The Sampler block of the input file is presented below. As with the regular [MonteCarloSampler](MonteCarloSampler.md), this block requires the `distributions` of the input parameters to the subApp. The [!param](/Samplers/ActiveLearningMonteCarloSampler/num_batch) parameter specifies the number of input samples to sample per step and also the number of allowed poor GP predictions before launching the high-fidelity model executions in parallel. The [!param](/Samplers/ActiveLearningMonteCarloSampler/flag_sample) is a Reporter object of type vector bool which indicates which GP predictions in the batch are poor to the Sampler.

!listing modules/stochastic_tools/test/tests/reporters/ActiveLearningGP/main_adam.i block=Samplers

### `ActiveLearningGaussianProcess` and `GaussianProcess`

The surrogate Trainers block of the input file is presented below. As with the regular [GaussianProcessTrainer](GaussianProcessTrainer.md), this block has the required parameters for training a GP model. The key difference with [ActiveLearningGaussianProcess](ActiveLearningGaussianProcess.md) is that it permits re-training of the GP model on the fly; this functionality is used by the `ActiveLearningGPDecision` Reporter. The below block presents [ActiveLearningGaussianProcess](ActiveLearningGaussianProcess.md) relying on Adam optimization for re-training the GP model.

!listing modules/stochastic_tools/test/tests/reporters/ActiveLearningGP/main_adam.i block=Trainers

The below block presents [ActiveLearningGaussianProcess](ActiveLearningGaussianProcess.md) relying on TAO optimization for re-training the GP model.

!listing modules/stochastic_tools/test/tests/reporters/ActiveLearningGP/main_adam.i block=Trainers

Once the GP model is re-trained, the `GaussianProcess` Surrogate is used by `ActiveLearningGPDecision` Reporter for making the GP predictions and uncertainty quantification. This block of the input file is presented below.

!listing modules/stochastic_tools/test/tests/reporters/ActiveLearningGP/main_adam.i block=Surrogates

### `ActiveLearningGPDecision`

The `Reporters` block of the input file is presented below. It requires the name of the [ActiveLearningMonteCarloSampler](ActiveLearningMonteCarloSampler.md) sampler. It also requires the names of the Reporter variables (which are all vectors) [!param](/Reporters/ActiveLearningGPDecision/flag_sample), [!param](/Reporters/ActiveLearningGPDecision/inputs), [!param](/Reporters/ActiveLearningGPDecision/gp_mean), and [!param](/Reporters/ActiveLearningGPDecision/gp_std) which, respectively, correspond to GP predictions being good/poor, input samples, GP mean predictions, and associated standard deviations. The values of these Reporters are all stored in an output json file.

In addition, the [!param](/Reporters/ActiveLearningGPDecision/n_train) parameter specifies the number of initial steps after which the GP model is first trained. [!param](/Reporters/ActiveLearningGPDecision/al_gp) specifies the name of [ActiveLearningGaussianProcess](ActiveLearningGaussianProcess.md) for facilitating GP re-training and [!param](/Reporters/ActiveLearningGPDecision/gp_evaluator) specifies the name of `GaussianProcess` for facilitating GP predictions and uncertainty quantification.

Finally, [!param](/Reporters/ActiveLearningGPDecision/learning_function) specifies the name of the acquisition (or learning) function. [!param](/Reporters/ActiveLearningGPDecision/learning_function_parameter) specifies a parameter for the acquisition (or learning) function; it may or may not be required depending upon the type of the acquisition (or learning) function used. For example, the U acquisition (or learning) function requires [!param](/Reporters/ActiveLearningGPDecision/learning_function_parameter) but the COV acquisition (or learning) function does not. [!param](/Reporters/ActiveLearningGPDecision/learning_function_threshold) specifies the acceptable threshold for the acquisition (or learning) function beyond which the GP predictions are marked as poor.

!listing modules/stochastic_tools/test/tests/reporters/ActiveLearningGP/main_adam.i block=Reporters

## Output format

The recommended output format for using active learning is a json file. For each time step, the json file contains vectors of input samples, `need_sample` bool variables (indicating good/poor GP predictions), GP mean predictions, and associated standard deviations. The total number of samples equals [!param](/Samplers/ActiveLearningMonteCarloSampler/num_samples), including the [!param](/Reporters/ActiveLearningGPDecision/n_train) samples used for training. The total number of times `need_sample` value is `True` is the total samples for which GP re-training is required after the initial training.

!alert note title=Number of output samples
When processing the json file, the samples whose `need_sample` value is `True` need to be ignored to avoid repeated results. Therefore, the number of output samples with `need_sample` set to False should be equal to [!param](/Samplers/ActiveLearningMonteCarloSampler/num_samples), as any re-training step does not count as an accepted sample.

!syntax parameters /Reporters/ActiveLearningGPDecision

!syntax inputs /Reporters/ActiveLearningGPDecision

!syntax children /Reporters/ActiveLearningGPDecision
