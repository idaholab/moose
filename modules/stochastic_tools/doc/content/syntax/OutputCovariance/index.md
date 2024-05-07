# OutputCovariance System

## Overview

The multi-output Gaussian process (MOGP) require an output covariance function to capture the correlations between the vector outputs. These output covariance functions can be defined in the `[OutputCovariance]` block. Correlations between the inputs are also required by the MOGP surrogate. Their functionality is similar to that described in [Covariance/index.md].

## Creating a OutputCovariance Function

A covariance function is created by inheriting from `OutputCovarianceFunctionBase` and overriding the methods in the base class.

## Using a OutputCovariance Function

#### In the MultiOutputGaussianProcess

The [/utils/MultiOutputGaussianProcess.md] is a class which incorporates the necessary data structures and
functions to create, train, and use MOGPs. One of the most important members
of this handler class is the output covariance function:

!listing stochastic_tools/include/utils/MultiOutputGaussianProcess.h line=OutputCovarianceBase *

The covariance function can be initialized in the handler by following the examples
given in [source description](/utils/MultiOutputGaussianProcess.md). Objects like
[/trainers/MultiOutputGaussianProcessTrainer.md] or [/surrogates/MultiOutputGaussianProcessSurrogate.md] can then access the
output covariance function through the handler class. These objects can also access the covariance between the inputs using [/utils/MultiOutputGaussianProcess.md].

#### OutputCovarianceInterface

Alternatively, by inheriting from
`OutputCovarianceInterface`, the child classes can easily fetch covariance functions
using the helper functions. Good examples are the [MultiOutputGaussianProcessTrainer.md] and
[/surrogates/MultiOutputGaussianProcessSurrogate.md] which utilize the helper functions to link an input
covariance function to the [MultiOutputGaussianProcess.md]:

!listing stochastic_tools/src/trainers/MultiOutputGaussianProcessTrainer.C start=_mogp_handler.initialize( end=}

## Example Input File Syntax

!listing test/tests/surrogates/multioutput_gp/mogp.i block=OutputCovariance

!syntax list /OutputCovariance objects=True actions=False subsystems=False

!syntax list /OutputCovariance objects=False actions=False subsystems=True

!syntax list /OutputCovariance objects=False actions=True subsystems=False
