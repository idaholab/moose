# Active learning Gaussian Process Decision

!syntax description /Reporters/ActiveLearningGPDecision

## Description

The `ActiveLearningGPDecision` is a Reporter class that performs the following functions to facilitate the active learning process in a parallelized fashion:

- For every new batch of input samples produced by the `ActiveLearningMonteCarloSampler`, the corresponding Gaussian Process (GP) predictions are made.
- These GP predictions are assessed for their quality by using the uncertainty information. If any of the predictions are of poor quality in the batch, those samples are marked and this information is sent back to the `ActiveLearningMonteCarloSampler`.
- Once a user-specified batch size of inputs for running the full model evaluations (i.e., those input samples for which the GP predictions are poor) is met and these evaluations are executed in parallel, re-train the GP model via the `ActiveLearningGaussianProcess` Surrogate class.
- Output the GP mean predictions and standard deviations and corresponding inputs to a json file. Also indicate those input samples for which a full model evaluation was necessary during the active learning.

## Why Gaussian Process surrogate for active learning?

A GP is a Bayesian surrogate model. This means, in addition to making predictions of the full model outputs, a GP produces uncertainty estimates of its outputs. Higher the uncertainty, lesser the confidence in the GP predictions. An illustration of the GP predictions and associated uncertainties is presented in [!ref](al1_sch). In addition, many studies (e.g., [!cite](xiu2019al) and [!cite](dhulipala2022al)) have found that the uncertainty estimates produced by a GP are robust. This in-built uncertainty information permits us to create an active feedback loop between the training data and GP in that we can use the GP uncertainty estimates to select the next best training data point(s). As such, we can even start the active learning process using a GP that is trained with little data.

!media GP_optimization.svg style=width:50%; id=al1_sch caption=Schematic of active learning in Monte Carlo simulations with parallel computing. The interaction between the three objects, `ActiveLearningMonteCarloSampler`, `ActiveLearningGaussianProcess`, and `ActiveLearningGPDecision`, is presented.

Of the spectrum of Bayesian surrogate models, a GP is the most straight-forward to train. First, during active learning, the training data set size would typically be small (in the order of hundreds) for most problems. Therefore, re-training the GP should be associated with a low computational cost. Even in those extreme cases where the training data set size becomes large, we can adopt stochastic optimization methods like Adam optimization [!cite](kingma2014adam) (this feature is available in MOOSE) to select a mini-batch of the training data set and substatially reduce the computational expense.

Owing to these reasons, a GP surrogate is usually preferred for active learning. On the other hand, there are some studies that have used other surrogates like a polynomial chaos expansion for active learning (e.g., [!cite](marelli2019al)).

## Procedure for active learning in MOOSE

In general, active learning has two phase: (1) initial training phase for the GP; and (2) the active learning phase.

[!ref](al2_sch) presents the initial training phase of the GP surrogate. The `ActiveLearningMonteCarloSampler` sends a batch of input samples for full model executions to the `SamplerFullSolveMultiApp`. These full models executions are performed in parallel separate sets of processors. Outputs from the model executions will be collected by the `ActiveLearningGPDecision` reporter. Once a required number of training data size is reached, `ActiveLearningGPDecision` will call the `ActiveLearningGaussianProcess` to initially train the GP model.

!media ActiveLearning_training.svg style=width:50%; id=al2_sch caption=Schematic of the initial training phase of the GP surrogate.

[!ref](al3_sch) presents the subsequent active learning phase. The `ActiveLearningMonteCarloSampler` sends a batch of input samples to the `ActiveLearningGPDecision` reporter. Then, the `ActiveLearningGaussianProcess` is called to make GP predictions. In addition, `ActiveLearningGPDecision` assesses the quality of the GP predictions through user-defined acquistion (or learning) functions. Some popular acquisition functions are currently available, and it is easy to include additional functions in the `ActiveLearningGPDecision` class. If the one or more of the GP predictions are poor (as indicated by the acquistion function), the corresponding input samples are marked and this information is communicated to the `ActiveLearningMonteCarloSampler`. This process is continues until a user-defined batch size is met for the maximum number of poor quality GP predictions. Once this threshold is reached, the `ActiveLearningMonteCarloSampler` sends a batch of those book-marked input samples whose GP predictions are poor for full model evaluations. These batch of full models can be run in parallel. Then, the full model outputs are acquired by the `ActiveLearningGPDecision` reporter where the GP model is re-trained. This process then repeats.

!media ActiveLearning_consolidated.svg style=width:50%; id=al3_sch caption=Schematic of active learning in Monte Carlo simulations with parallel computing. The interaction between the three objects, `ActiveLearningMonteCarloSampler`, `ActiveLearningGaussianProcess`, and `ActiveLearningGPDecision`, during the active learning phase.

## Input file syntax

The full input file driver for active learning is presented below. The three main components of the input file are `ActiveLearningMonteCarloSampler`, `ActiveLearningGaussianProcess`, and `ActiveLearningGPDecision`. These are discussed in detail below.

!listing modules/stochastic_tools/test/tests/reporters/ActiveLearningGP/Main_adam.i

### `ActiveLearningMonteCarloSampler`

The Sampler block of the input file is presented below. As with the regular [MonteCarloSampler](MonteCarloSampler.md), this block requires the `distributions` of the input parameters to the subApp. The `num_batch` parameter specifies the number of inputs samples to sample per step and also the number of allowed poor GP predictions before launching the full model executions in parallel. The `flag_sample` is a Reporter object of type vector bool which indicates which GP predictions in the batch are poor to the Sampler.

!listing modules/stochastic_tools/test/tests/reporters/ActiveLearningGP/Main_adam.i block=Samplers

### `ActiveLearningGaussianProcess` and `GaussianProcess`

The surrogate Trainers block of the input file is presented below. As with the regular [GaussianProcessTrainer](GaussianProcessTrainer.md), this block has the required parameters for training a GP model. The key difference with `ActiveLearningGaussianProcess` is that it permits re-training of the GP model on the fly; this functionality is used by the `ActiveLearningGPDecision` Reporter. The below block presents `ActiveLearningGaussianProcess` relying on Adam optimization for re-training the GP model. 

!listing modules/stochastic_tools/test/tests/reporters/ActiveLearningGP/Main_adam.i block=Trainers

The below block presents `ActiveLearningGaussianProcess` relying on TAO optimization for re-training the GP model.

!listing modules/stochastic_tools/test/tests/reporters/ActiveLearningGP/Main_tao.i block=Trainers

Once the GP model is re-trained, the `GaussianProcess` Surrogate is used by `ActiveLearningGPDecision` Reporter for making the GP predictions and uncertainty quantification. This block of the input file is presented below.

!listing modules/stochastic_tools/test/tests/reporters/ActiveLearningGP/Main_adam.i block=Surrogates

### `ActiveLearningGPDecision`

The Reporters block of the input file is presented below. It requires the name of the `ActiveLearningMonteCarloSampler` sampler. It also requires the names of the Reporter variables (which are all vectors) `flag_sample`, `inputs`, `gp_mean`, and `gp_std` which, respectively, correspond to GP predictions being good/poor, input samples, GP mean predictions, and associated standard deviations. These values of these Reporters are all stored in an output json file.

In addition, the `n_train` parameter specifies the number of initial steps after which the GP model is first trained. `al_gp` specifies the name of `ActiveLearningGaussianProcess` for facilitating GP re-training and `gp_evaluator` specifies the name of `GaussianProcess` for facilitating GP predictions and uncertainty quantification.

Finally, `learning_function` specifies the name of the acquisition (or learning) function. `learning_function_parameter` specifies a parameter for the acquisition (or learning) function; it may or may not be required depending upon the type of the acquisition (or learning) function used. For example, the U acquisition (or learning) function requires `learning_function_parameter` but the COV acquisition (or learning) function does not require it. `learning_function_threshold` specifies the acceptable threshold for the acquisition (or learning) function beyond which the GP predictions are marked as poor.

!listing modules/stochastic_tools/test/tests/reporters/ActiveLearningGP/Main_adam.i block=Reporters

## Output format

The recommended output format for using active learning is a json file. For each time step, the json file contains vectors of input samples, `need_sample` bool variables (indicating good/poor GP predictions), GP mean predictions, and associated standard deviations. The total number of samples equal the prescribed time steps times the `num_batch` specified the Samplers block. The total number of times `need_sample` value is True is the total samples for which GP re-training is required after the initial training.

!alert note title=Effective number of output samples
When processing the json file, those samples whose `need_sample` value is True needs to be ignored to avoid repeated results. Therefore, the effective number of output samples is prescribed time steps times the `num_batch` specified the Samplers block minus the number of times `need_sample` value is True. 

!syntax parameters /Reporters/ActiveLearningGPDecision

!syntax inputs /Reporters/ActiveLearningGPDecision

!syntax children /Reporters/ActiveLearningGPDecision