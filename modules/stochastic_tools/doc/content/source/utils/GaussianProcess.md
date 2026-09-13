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

### Applying a link function

Before setting up the covariance matrix, training data can be transformed through
a [GPLinkFunction.md] to enforce inequality constraints. The link type and bounds
are configured through [GaussianProcessTrainer.md]:

!listing stochastic_tools/src/trainers/GaussianProcessTrainer.C
         start=// ---- Link function ----
         end=// Apply link function to training data

The transformed values are standardized and the GP trains entirely in latent space.
At prediction time the inverse link is applied to recover physical outputs.

### Derivative observations

Monotonicity constraints are encoded via virtual derivative observations appended
to the training set. The `GaussianProcess` object stores the virtual parameter
locations and their associated derivative dimensions, and assembles an augmented
covariance matrix at setup time:

!listing stochastic_tools/src/trainers/GaussianProcessTrainer.C
         start=// ---- Derivative constraint (virtual observations) ----
         end=// ---- Penalty constraints ----

### Penalty constraints

Soft bounds on predicted means at specified evaluation points are enforced by
adding a penalty term to the NLML loss and gradient during Adam optimization.
The penalty is computed inside `getLoss` and `getGradient` in `GaussianProcess.C`.

