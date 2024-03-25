# MultiOutputGaussianProcessHandler

## Overview

The MultiOutputGaussianProcessHandler is designed to incorporate structures (data) and
functions commonly used by every object that needs to use/modify Multi-Output Gaussian processes (MOGPs) such as [/trainers/MultiOutputGaussianProcessTrainer.md], [/surrogates/MultiOutputGaussianProcess.md], or Active Learning objects. It contains accesses to the input and outputs covariance functions, stores covariance matrices and their corresponding decomposition and inverse action.

### Initializing

The object which requires access to MOGP-related functionalities shall
have it as a member variable, which is important to enable restarting capabilities:

!listing stochastic_tools/include/trainers/MultiOutputGaussianProcessTrainer.h line=StochasticTools::MultiOutputGaussianProcessHandler & _mogp_handler;

An important step is the initialization, which can either be done using the `initialize` function as in [/trainers/MultiOutputGaussianProcessTrainer.md]:

!listing stochastic_tools/src/trainers/MultiOutputGaussianProcessTrainer.C start=_mogp_handler.initialize(
                                  end=}

Or by declaring it as in [/surrogates/MultiOutputGaussianProcess.md]:

!listing /surrogates/MultiOutputGaussianProcess.C line=declareModelData

### Creating input and output covariance matrices

Once the handler has been linked, one can use it to create a covariance matrix by either using `setupCovarianceMatrix` as in [/trainers/MultiOutputGaussianProcessTrainer.md]:

!listing stochastic_tools/src/trainers/MultiOutputGaussianProcessTrainer.C start=_mogp_handler.setupCovarianceMatrix(
                                  end=}

Or by simply calling the input covariance matrix builder as in [/surrogates/MultiOutputGaussianProcess.md] for the input covariance:

!listing /surrogates/MultiOutputGaussianProcess.C line=computeCovarianceMatrix

and the output covariance matrix builder as in [/surrogates/MultiOutputGaussianProcess.md] for the output covariance:

!listing /surrogates/MultiOutputGaussianProcess.C start=_mogp_handler.getB(
                                  end=}

### Optimizing hyper parameters

As described in [/trainers/MultiOutputGaussianProcessTrainer.md], the input and output covariance functions will have hyper parameters that need to be optimized to be able to get accurate predictions from the corresponding MOGP. We can optimize these
parameters during with Adam (stochastic algorithm):

!listing stochastic_tools/src/utils/MultiOutputGaussianProcessHandler.C start=tuneHyperParamsAdam(training_params end=);