# Covariance System

## Overview

Some surrogate models allow for a covariance function (often referred to as kernel functions or simply kernels) to project the input parameters into a different input space. The Gaussian process surrogate is an example of such a surrogate. These covariance functions can be defined in the `[Covariance]` block.

## Creating a Covariance Function

A covariance function is created by inheriting from `CovarainceFunctionBase` and overriding the methods in the base class.

## Using a Covariance Function

#### In a Trainer

Within the surrogate/trainer framework in the Stochastic Tools module, the covariance function will often be used within the trainer object. The trainer should inherit from the `CovarianceInterface` class to enable determining the covariance object by name.

!listing GaussianProcessTrainer.h line=class GaussianProcessTrainer

Because many trainers which use a covariance function should be compatible with many different types of covariance functions, polymorphism of C++ pointers are often used.

!listing GaussianProcessTrainer.h line=CovarianceFunctionBase *

The covariance can be found by name using the interface

!listing GaussianProcessTrainer.C line=covariance_function(

#### In a Surrogate

The covariance function may also be used in the surrogate. For convenience the surrogate is able to determine the correct covariance used in the training step.

If the surrogate is given the trainer (not loaded from a file) the covariance function can be linked via the trainer object.

!listing GaussianProcess.C start=covariance_function( end=: nullptr) include-end=True

If the surrogate loads the training data from a file, the [](LoadCovarianceDataAction.md) automatically reconstructs the covariance object used in the training phase, and calls the surrogate `setupCovariance()` method to make the linkage. This recreation is done by storing the `buildHyperParamMap()` in the trainer, and storing the hyperparameters for use in the surrogate.


## Example Input File Syntax

!listing test/tests/surrogates/gaussian_process/GP_squared_exponential_training.i block=Covariance

!syntax list /Covariance objects=True actions=False subsystems=False

!syntax list /Covariance objects=False actions=False subsystems=True

!syntax list /Covariance objects=False actions=True subsystems=False
