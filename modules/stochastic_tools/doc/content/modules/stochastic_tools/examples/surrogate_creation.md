# Creating a Surrogate Model

This example goes through the process of creating a custom surrogate model, in his case the creation of [NearestPointSurrogate.md].

## Overview

Building a surrogate model requires the creation of two objects: SurrogateTrainer and SurrogateModel. The SurrogateTrainer uses information from samplers and results to construct variables to be saved into a `.rd` file at the conclusion of the training run. The SurrogateModel object loads the data from the `.rd` and contains a function called `evaluate` that evaluates the surrogate model at a given input. The SurrogateTrainer and Surrogate are heavily tied together where each have the same member variables, the difference being one saves the data and the other loads it. It might be beneficial to have an interface class that contains common functions for training and evaluating, to avoid duplicate code. This example will not go into the creation of this interface class.

## Creating a Trainer

This example will go over the creation of [NearestPointTrainer](NearestPointTrainer.md). [Trainers](Trainers/index.md) are derived from `SurrogateTrainer` which performs a loop over the training data and calls virtual functions that derived classes are meant to override to perform the proper training.

This example makes use of `SurrogateTrainer` input parameters, API functions, and protected member variables for gathering and accessing common types of training data. For further details on these features, see [Trainers System](Trainers/index.md).

### validParams

The trainer requires the input of a sampler, so that it understands how many data points are included and how they are distributed across processors. The trainer also needs the predictor and response values from the full-order model which are stored in a [vector postprocessor](VectorPostprocessors/index.md) or [reporter](Reporters/index.md).

!listing SurrogateTrainer.C re=InputParameters\sSurrogateTrainer::validParams.*?^}

!listing NearestPointTrainer.C re=InputParameters\sNearestPointTrainer::validParams.*?^}

By default, classes inheriting from `SurrogateTrainer` support `Real` and `std::vector<Real>` response types. For responses of these types, a [Reporter](Reporters/index.md) can be specified using the [!param](/Trainers/PolynomialRegressionTrainer/response) and [!param](/Trainers/PolynomialRegressionTrainer/response_type) input parameters. Additionally, child classes can support predictor data as a combination of `Sampler` columns as well as `Reporter` values - these options can be controlled using the [!param](/Trainers/PolynomialRegressionTrainer/response) and [!param](/Trainers/PolynomialRegressionTrainer/response_type) input parameters.

Classes derived from `SurrogateTrainer` also support $k$-fold cross-validation. For details on the input parameters for this capability, see [Cross Validation](cross_validation.md). To ensure compatibility with these features, some extra considerations are needed during implementation of a new `Trainer`. These will be discussed in the following sections.

### Constructor

All trainers are based on SurrogateTrainer, which provides the necessary interface for saving the surrogate model data and gathering response/predictor. Any additional data meant to be saved and gathered is defined in the constructor of the training object. In [NearestPointTrainer](NearestPointTrainer.md), the variables `_sample_points` and `_sample_results` are declared as the necessary surrogate data, see [Trainers](Trainers/index.md) for more information on declaring model data. Because we will use several default inputs to retrieve training data, we only need to resize these variables for the number of dimensions in the training data. Each processor will contain a portion of the samples and results. We will gather all the samples in `postTrain()`.

!listing NearestPointTrainer.C re=NearestPointTrainer::NearestPointTrainer.*?^}

The member variables `_sample_points`, `_sample_results`, and `_predictor_row` are defined in the header file:

!listing NearestPointTrainer.h start=protected end=}; include-start=False

### preTrain

`preTrain()` is called before the sampler loop. This is typically used to initialize variables and allocate memory. For [NearestPointTrainer.md], we will explicitly clear `_sample_points` and `_sample_results`. This is done because the implementation of $k$-fold cross-valdiation used in `SurrogateTrainer` requires that `preTrain()` resets the state of the trainer and clears any essential data related to prior training sets (for further details, see [Trainers System](Trainers/index.md)).

!listing NearestPointTrainer.C re=void\sNearestPointTrainer::preTrain.*?^}

### train

`train()` is where the actual training occurs. This function is called during the sampler loop for each row, at which time the member variables `_row`, `_local_row`, and ones gathered with `getTrainingData` are updated. In [NearestPointTrainer.md], we will `push_back` predictor and response data as appropriate. Using `push_back` is convenient because some samples may be skipped (due to non-convergence, or intentionally during cross-validation).

!listing NearestPointTrainer.C re=void\sNearestPointTrainer::train.*?^}

### postTrain

`postTrain()` is called after the sampler loop. This is typically where processor communication happens. Here, we use `postTrain()` to gather all the local `_sample_points` so that each processor has the full copy. `_communicator.allgather` makes it so that every processor has a copy of the full array and `_communicator.gather` makes it so that only one of the processors has the full copy, the latter is typically used because outputting only happens on the root processor. See [libMesh::Parallel::Communicator](http://libmesh.github.io/doxygen/classlibMesh_1_1Parallel_1_1Communicator.html) for more communication options.

!listing NearestPointTrainer.C re=void\sNearestPointTrainer::postTrain.*?^}

## Creating a Surrogate

This example will go over the creation of [NearestPointSurrogate](NearestPointSurrogate.md). [Surrogates](Surrogates/index.md) are a specialized version of a MooseObject that must have the `evaluate` public member function. The `validParams` for a surrogate will generally define how the surrogate is evaluated. [NearestPointSurrogate.md] does not have any options for the method of evaluation.

### Constructor

In the constructor, the references for the model data are defined, taken from the training data:

!listing NearestPointSurrogate.C re=NearestPointSurrogate::NearestPointSurrogate.*?^}

See [Surrogates](Surrogates/index.md) for more information on the `getModelData` function. `_sample_points` in the surrogate is a `const` reference, since we do not want to modify the training data during evaluation:

!listing NearestPointSurrogate.h start=/// Array end=sample_points include-end=true

### evaluate

`evaluate` is a public member function required for all surrogate models. This is where surrogate model is actually used. `evaluate` takes in parameter values and returns the surrogate's estimation of the quantity of interest. See [EvaluateSurrogate](EvaluateSurrogate.C) for an example on how the `evaluate` function is used.

!listing NearestPointSurrogate.C re=Real\sNearestPointSurrogate::evaluate.*?^}

!listing NearestPointSurrogate.C re=unsigned int\sNearestPointSurrogate::findNearestPoint.*?^}
