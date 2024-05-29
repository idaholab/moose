# GaussianProcess

## Overview

The GaussianProcess is designed to incorporate structures (data) and
functions commonly used by every object that needs to use/modify Gaussian Processes
such as [GaussianProcessTrainer.md], [GaussianProcessSurrogate.md], or
Gaussian Process-based active learning objects. It contains accesses
to the covariance function, stores covariance matrices
and their corresponding decomposition and inverse action.

### Initializing

The object which requires access to Gaussian Process-related functionalities shall
have it as a member variable, which is important to enable restarting capabilities:

!listing stochastic_tools/include/trainers/GaussianProcessTrainer.h line=StochasticTools::GaussianProcess & _gp;

An important step is the initialization of the covariance function, which can
either be done using the `initialize` function as in [GaussianProcessTrainer.md]:

!listing stochastic_tools/src/trainers/GaussianProcessTrainer.C start=_gp.initialize(
                                  end=}

Or by linking it directly as in [GaussianProcessSurrogate.md]:

!listing /surrogates/GaussianProcessSurrogate.C line=linkCovarianceFunction

### Creating a covariance matrix

Once the covariance function has been added to the handler, one can use it to
create a covariance matrix by either using `setupCovarianceMatrix` as in
[GaussianProcessTrainer.md]:

!listing stochastic_tools/src/trainers/GaussianProcessTrainer.C start=_gp.setupCovarianceMatrix(
                                  end=}

Or by simply calling the covariance matrix builder as in [GaussianProcessSurrogate.md]:

!listing /surrogates/GaussianProcessSurrogate.C line=computeCovarianceMatrix

### Optimizing hyper parameters

As described in [GaussianProcessTrainer.md], the covariance function might
have hyper parameters that need to be optimized to be able to get accurate
predictions from the corresponding Gaussian Processes. We can optimize these
parameters during the generation of the Covariance matrix:

!listing stochastic_tools/src/trainers/GaussianProcessTrainer.C start=_gp.setupCovarianceMatrix( end=}

Or by simply calling the optimizer with Adam (stochastic algorithm):

!listing stochastic_tools/src/utils/GaussianProcess.C start=tuneHyperParamsAdam(training_params end=);

