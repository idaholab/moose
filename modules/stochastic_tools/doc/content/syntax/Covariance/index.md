# Covariance System

## Overview

Some surrogate models allow for a covariance function (often referred to as kernel functions or simply kernels) to project the input parameters into a different input space. The Gaussian process surrogate is an example of such a surrogate. These covariance functions can be defined in the `[Covariance]` block.

## Creating a Covariance Function

A covariance function is created by inheriting from `CovarianceFunctionBase` and overriding the methods in the base class.

## Using a Covariance Function

#### In the GaussianProcess

The [GaussianProcess.md] is a class which incorporates the necessary data structures and
functions to create, train, and use Gaussian Processes. One of the most important members
of this handler class is the covariance function:

!listing stochastic_tools/include/utils/GaussianProcess.h line=CovarianceFunctionBase *

The covariance function can be initialized in the handler by following the examples
given in [source description](GaussianProcess.md). Objects like
[GaussianProcessTrainer.md] or [GaussianProcess.md] can then access the
covariance function through the handler class.

#### CovarianceInterface

Alternatively, by inheriting from
`CovarianceInterface`, the child classes can easily fetch covariance functions
using the helper functions. Good examples are the [GaussianProcessTrainer.md] and
[GaussianProcess.md] which utilize the helper functions to link an input
covariance function to the [GaussianProcess.md]:

!listing stochastic_tools/src/trainers/GaussianProcessTrainer.C start=_gp.initialize( end=}

#### In a Surrogate

If the surrogate loads the training data from a file, the [LoadCovarianceDataAction.md]
automatically reconstructs the covariance object used in the training phase, and
calls the surrogate `setupCovariance()` method to make the linkage. This recreation
is done by storing the hyper parameter map in the Gaussian Process handler of the
trainer for use in the surrogate.

#### Dependencies between Covariance objects

Some covariance functions may depend on other covariance functions for adding
complexity. A typical scenario can be a Gaussian Process built using the
[Linear Model of Coregionalization](LMC.md) that predicts multiple
outputs at once by handling the covariance between the outputs as well.


## Example Input File Syntax

!listing test/tests/surrogates/gaussian_process/GP_squared_exponential_training.i block=Covariance

!syntax list /Covariance objects=True actions=False subsystems=False

!syntax list /Covariance objects=False actions=False subsystems=True

!syntax list /Covariance objects=False actions=True subsystems=False
