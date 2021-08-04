# Evaluating a Surrogate Model

This example demonstrates how to evaluate a trained surrogate model. [NearestPointSurrogate.md] is used as the example surrogate model. See [Creating a surrogate model](/examples/surrogate_creation.md) for details on how this surrogate is created. See [Training a surrogate model](/examples/surrogate_training.md) for details on how to train a surrogate model.

## Overview

In general, a [SurrogateModel](Surrogates/index.md) object takes in training data in the form of `.rd` files or the name of the training object itself. The former performs training and evaluating in two separate steps, the latter performs them both in a single step. Since most practical applications perform training once and evaluation in multiple instances, this example focuses on the two step method. Every [SurrogateModel](Surrogates/index.md) object has a public member function, `evaluate`, that gives the surrogate's estimate to a quantity of interest with the parameters as input. Some specialized surrogates, like [PolynomialChaos.md], have more functions for computing statistics. However, this example will focus on the simple [NearestPointSurrogate.md], which only uses the `evaluate` function.

!include surrogate_training.md start=heat_conduction_model_begin end=heat_conduction_model_finish

## Evaluation

This section shows how to set up an input file to load and evaluate a surrogate model. The training data was created with the steps from [Training a surrogate model](/examples/surrogate_training.md). To demonstrate the usefulness of a surrogate model, the model will be evaluated using [Monte Carlo](MonteCarloSampler.md) sampling with parameter distributions described in the previous section. The results of this sampling will be used to compute statistical moments and produce probability distributions.

!include surrogate_training.md start=omitting_solve_begin end=omitting_solve_finish replace=['nearest_point_training', 'nearest_point_uniform']

### Surrogate Model

The surrogate model is loaded by inputting the training data file with the [!param](/Surrogates/NearestPointSurrogate/filename) parameter. In this example, two surrogates are loaded with two different training data files for average temperature and maximum temperature.

!listing examples/surrogates/nearest_point_uniform.i block=Surrogates

### Defining a Sampler

In this example, the surrogate is evaluated at points given by a sampler. Here we use the [MonteCarloSampler](MonteCarloSampler.md) to generate random points defined by a [Uniform](Uniform.md) or a [Normal](Normal.md) distribution for each parameter. See [Example 1: Monte Carlo](/examples/monte_carlo.md) for more details on setting up this sampler.

!listing examples/surrogates/nearest_point_uniform.i block=Distributions caption=Uniform distribution for each parameter

!listing examples/surrogates/nearest_point_normal.i block=Distributions caption=Normal distribution for each parameter

!listing examples/surrogates/nearest_point_uniform.i block=Samplers caption=Monte Carlo sampler

### Sampling Surrogate

Evaluating a surrogate model occurs within objects that obtain the surrogate object's reference and call the `evaluate` function. In this example, we will use [EvaluateSurrogate.md] to evaluate the surrogate. [EvaluateSurrogate.md] takes in a sampler and the surrogate model as inputs, and evaluates the surrogate at the points given by the sampler.

!listing nearest_point_uniform.i block=samp

The results of evaluating the surrogate can then be used to compute statistics like mean and standard deviation:

!listing nearest_point_uniform.i block=stats

### Results

The results of the of inputs from the previous sections produce csv files of the evaluation data. These files can then be used to produce probability distributions like in [uniform] and [normal]. Even for this simple model problem, evaluating the surrogate was orders of magnitude faster with significantly less memory consumption.

!media nearest_point_uniform_hist.svg id=uniform caption=Temperature distributions with uniform parameter distribution

!media nearest_point_normal_hist.svg id=normal caption=Temperature distributions with normal parameter distribution
