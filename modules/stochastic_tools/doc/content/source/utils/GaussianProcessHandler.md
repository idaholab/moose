# GaussianProcessHandler

## Overview

The GaussianProcessHandler is designed to incorporate structures (data) and
functions commonly used by every object that needs to use/modify Gaussian Processes
such as [GaussianProcessTrainer.md], [GaussianProcess.md], or Active
Learning objects. It contains accesses to the covariance function, stores covariance matrices
and their corresponding decomposition and inverse action.

### Initializing

The object which requires access to Gaussian Process-related functionalities shall
have it as a member variable, which is important to enable restarting capabilities:

!listing GaussianProcessTrainer.h line=StochasticTools::GaussianProcessHandler & _gp_handler;

An important step is the initialization of the covariance function, which can
either be done using the `initialize` function as in [GaussianProcessTrainer.md]:

!listing GaussianProcessTrainer.C start=_gp_handler.initialize(
                                  end=}

Or by linking it directly as in [GaussianProcess.md]:

!listing /surrogates/GaussianProcess.C line=linkCovarianceFunction

### Creating a covariance matrix

Once the covariance function has been added to the handler, one can use it to
create a covariance matrix by either using `setupCovarianceMatrix` as in
[GaussianProcessTrainer.md]:

!listing GaussianProcessTrainer.C start=_gp_handler.setupCovarianceMatrix(
                                  end=}

Or by simply calling the covariance matrix builder as in [GaussianProcess.md]:

!listing /surrogates/GaussianProcess.C line=computeCovarianceMatrix

### Optimizing hyper parameters

As described in [GaussianProcessTrainer.md], the covariance function might
have hyper parameters that need to be optimized to be able to get accurate
predictions from the corresponding Gaussian Processes. We can optimize these
parameters during the generation of the Covariance matrix:

!listing GaussianProcessTrainer.C start=_gp_handler.setupCovarianceMatrix( end=}

Or by simply calling the optimizer with TAO (deterministic algorithms):

!listing GaussianProcessHandler.C start=tuneHyperParamsTAO end=)) include-end=True

or with Adam (stochastic algorithm):

!listing GaussianProcessHandler.C start=tuneHyperParamsAdam(training_params end=);

