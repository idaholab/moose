# Training a Surrogate Model

This example goes over how to train a surrogate model using the [Trainers](Trainers/index.md) input syntax. For demonstrative purpose, this example uses the [NearestPointTrainer](NearestPointTrainer.md), see [Creating a surrogate model](/examples/surrogate_creation.md) for information on how this model was created.

## Overview

In general, a [SurrogateTrainer](Trainers/index.md) object takes in input and output data from a full-order model to create reduced-order model that is later evaluated by a [SurrogateModel](Surrogates/index.md) object to quickly emulate the full-order model outputs. The advantage is that creating a reduced-order generally requires a minimal amount of samples, so taking a large set of samples for statistical quantities is much faster in the end when using the surrogate system. To demonstrate the training process, we use a heat conduction model as the full-order model with maximum and average temperatures as the quantities of interest. The reduced-order model uses [NearestPointTrainer](NearestPointTrainer.md).

!! heat_conduction_model_begin

## Model Problem

This example uses a one-dimensional heat conduction problem as the full-order model which has certain uncertain parameters. The model equation is as follows:

!equation
-k\frac{d^2T}{dx^2} = q \,, \quad x\in[0, L]

!equation
\begin{aligned}
\left.\frac{dT}{dx}\right|_{x=0} &= 0 \\
T(x=L) &= T_{\infty}
\end{aligned}

The quantities of interest are the average and maximum temperature:

!equation
\begin{aligned}
\bar{T} &= \frac{\int_{0}^{L}T(x)dx}{L} \\
T_{\max} &= \max_{x\in[0,L]}T(x)
\end{aligned}

### Parameter Uncertainty

For demonstration, each of these parameters will have two types of probability distributions: [Uniform](Uniform.md) ($\mathcal{U}(a,b)$) and [Normal](Normal.md) ($\mathcal{N}(\mu,\sigma)$). Where $a$ and $b$ are the max and minimum bounds of the uniform distribution, respectively. And $\mu$ and $\sigma$ are the mean and standard deviation of the normal distribution, respectively.

The uncertain parameters for this model problem are:

!table
| Parameter | Symbol | Uniform | Normal |
| :- | - | - | - |
| Conductivity | $k$ | $\sim\mathcal{U}(1, 10)$ | $\sim\mathcal{N}(5, 2)$ |
| Volumetric Heat Source | $q$ | $\sim\mathcal{U}(9000, 11000)$ | $\sim\mathcal{N}(10000, 500)$ |
| Domain Size | $L$ | $\sim\mathcal{U}(0.01, 0.05)$ | $\sim\mathcal{N}(0.03, 0.01)$ |
| Right Boundary Temperature | $T_{\infty}$ | $\sim\mathcal{U}(290, 310)$ | $\sim\mathcal{N}(300, 10)$ |

### Analytical Solutions

This simple model problem has analytical descriptions for the field temperature, average temperature, and maximum temperature:

!equation
\begin{aligned}
T(x,k,q,L,T_{\infty}) &= \frac{q}{2k}\left(L^2 - x^2\right) + T_{\infty} \\
\bar{T}(k,q,L,T_{\infty}) &= \frac{qL^2}{3k} + T_{\infty} \\
T_{\max}(k,q,L,T_{\infty}) &= \frac{qL^2}{2k} + T_{\infty}
\end{aligned}

With the quadratic feature of the field temperature, using quadratic elements in the discretization will actually yield the exact solution.

### Input File

Below is the input file used to solve the one-dimensional heat conduction model.

!listing examples/surrogates/sub.i

With this input the uncertain parameters are defined as:

1. $k\rightarrow$ `Materials/conductivity/prop_values`
1. $q\rightarrow$ `Kernels/source/value`
1. $L\rightarrow$ `Mesh/xmax`
1. $T_{\infty}\rightarrow$ `BCs/right/value`

These values in the `sub.i` file are arbitrary since the stochastic master app will be modifying them.

!! heat_conduction_model_finish

## Training

This section describes how to set up an input file to train a surrogate and output the training data. This file will act as the master app and use the [MultiApps](MultiApps/index.md) and [Transfers](Transfers/index.md) systems to communicate and run the [sub app](examples/surrogates/sub.i).

!! omitting_solve_begin

### Omitting Solve

Any input file in [MOOSE] needs to include a [Mesh](Mesh/index.md), [Variables](syntax/Variables/index.md), and [Executioner](Executioner/index.md) block. However, the stochastic master app does not actually create or solve a system. So the [StochasticToolsAction](StochasticTools/index.md) builds a minimal model to satisfy these requirements:

!listing examples/surrogates/nearest_point_training.i block=StochasticTools

!! omitting_solve_finish

### Training Sampler

First, a sampler needs to be created, which defines the training points for which the full-order model will be run. In this example, the training points are defined by the [CartesianProductSampler.md]:

!listing examples/surrogates/nearest_point_training.i block=Samplers

The [!param](/Samplers/CartesianProduct/linear_space_items) parameter defines a uniform grid of points. Having 10 points for each parameter results in 10,000 training points. [!param](/Samplers/CartesianProduct/execute_on) needs to be set to `PRE_MULTIAPP_SETUP` for this example since we will be using [MultiAppSamplerControl.md] to perturb the parameters.

### MultiApp and Control

[SamplerFullSolveMultiApp.md] is used to setup the communication between the master and sub app with the ability to loop over samples. [MultiAppSamplerControl.md] defines which parameters to perturb, which =must= correspond with the dimensionality defined in the [Sampler](Samplers/index.md).

!listing examples/surrogates/nearest_point_training.i block=MultiApps Controls

[MultiAppSamplerControl.md] is not the only method to perturb sub app input parameters. If all the parameters are controllable, then [SamplerParameterTransfer.md] can be used. See the [Monte Carlo Example](/examples/monte_carlo.md) for more details.

### Transferring Results

After each perturbed simulation of the sub app is finished, the results are transferred to the master app. [SamplerReporterTransfer.md] is used to transfer postprocessors, vectorpostprocessors, or reporter values from the sub app to a [StochasticReporter.md] in the master app.

!listing examples/surrogates/nearest_point_training.i block=Transfers Reporters

Here, there are two transfers that transfer the sub app postprocessors computing $\bar{T}$ and $T_{\infty}$ to two different master app reporters. The [!param](/Reporters/StochasticReporter/outputs) parameter is set to `none` so it doesn't output the unnecessary results to a CSV file. The trainer will directly grab the data from the reporters.

### Training Object

Training objects take in sampler and results data to train the surrogate model. The sampler points are taken directly from the sampler by defining [!param](/Trainers/NearestPointTrainer/sampler). The results are taken directly from a reporter or vector postprocessor, the object is defined by [!param](/Trainers/NearestPointTrainer/response) and [!param](/Trainers/NearestPointTrainer/predictors).

!listing examples/surrogates/nearest_point_training.i block=Trainers

In the `nearest_point_avg` trainer, we see that the predictor values are from a VPP and sampler columns. In the `nearest_point_max` trainer, the predictor names were not specified, which indicates that all the sampler columns are used as predictors.
[StochasticReporter.md] defines the reporter name by the name of the transfer executing the transfer, which is why we know that the values names are `data` in this example.

### Outputting Training Data

All [training objects](Trainers/index.md) create variables that surrogates can use for evaluation. Saving this data will allow the surrogate to be run separately in another input. [SurrogateTrainerOutput.md] is used to output training data.

!listing examples/surrogates/nearest_point_training.i block=Outputs

The outputted file will be named: `<file_base>_<surrogate name>.rd`. For this example, the two outputted files are:

- `nearest_point_training_out_nearest_point_avg.rd`
- `nearest_point_training_out_nearest_point_max.rd`
