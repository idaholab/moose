# Trainers System

## Overview

Objects within the `[Trainers]` block are derived from `SurrogateTrainer` and
are designed for creating training data for use with a model (see [Surrogates/index.md]).

## Creating a SurrogateTrainer

To create a trainer the new object should inherit from `SurrogateTrainer`, which is derived
from [GeneralUserObject.md]. `SurrogateTrainer` overrides the `execute()` function to loop
through the rows of a given [sampler](Samplers/index.md), specified by the
[!param](/Trainers/NearestPointTrainer/sampler) parameter:

!listing SurrogateTrainer.C re=void\sSurrogateTrainer::execute.*?^}

!listing SurrogateTrainer.C re=void\sSurrogateTrainer::executeTraining.*?^}

The method will execute once per execution flag (see [SetupInterface.md]) on each processor.
There are three virtual functions that derived class can and should override:

!listing SurrogateTrainer.h start=protected: end=postTrain include-start=False include-end=True

- `preTrain()` is called before the sampler loop and is typically used for resizing variables for the given number of data points.
- `train()` is called within the sampler loop where member variables `_local_row`, `_row`, and those declared with `getTrainingData` are updated.
- `postTrain()` is called after the sampler loop and is typically used for MPI communication.

## Gathering Training Data

In order to ease the gathering the required data needed for training, `SurrogateTrainer`
includes API to get reporter data which takes care of the necessary size checks and
distributed data indexing.
The idea behind this is to emulate the element loop behavior in other MOOSE objects.
For instance, in a kernel, the value of _u corresponds to the solution in an element.
Here data referenced with `getTrainingData` will correspond to the the value of the
data in a sampler row. The returned reference is to be used in the `train()` function.
There are five functions that derived classes can call to gather training data:

!listing SurrogateTrainer.h start=TRAINING_DATA_BEGIN end=TRAINING_DATA_END include-start=False

- `getTrainingData<T>(const ReporterName & rname)` will get a vector of training data from a reporter value of type `std::vector<T>`, whose name is defined by `rname`.
- `getSamplerData()` will simply return a vector of the sampler row.
- `getPredictorData()` will return a vector of predictor data, including values from `Reporters` specified using the common [!param](/Trainers/NearestPointTrainer/predictors) input parameter.
- `getCurrentSampleSize()` and `getLocalSampleSize()` will return global and local sample sizes, recalculated periodically when samples are intentionally excluded (for example, during cross validation).

## Accessing Common Types of Training Data

Many of the `Trainers` in the module share common characteristics:

- They support specifying `Reporters` as predictor data using the [!param](/Trainers/NearestPointTrainer/predictors) input parameter, and choosing column subsets from the `Sampler` using the [!param](/Trainers/NearestPointTrainer/predictor_cols) input parameter.
- They support `Real` and/or `std::vector<Real>` types for response data. The input parameters [!param](/Trainers/NearestPointTrainer/response) and [!param](/Trainers/NearestPointTrainer/response_type) are used to declare a `Reporter`, and the type of that reporter, to handle these two cases.

Although the `getTrainingData` API should generally be used in the constructor of the derived `Trainer` class to gather required training data, it is anticipated that most trainers will need similar capabilities. To facilitate these use cases, `SurrogateTrainer` provides input parameters and protected member variables to gather and access training data of the types defined above.

!listing SurrogateTrainer.C start=// Common Training Data end=// End Common Training Data

!listing SurrogateTrainer.h start=// TRAINING_DATA_MEMBERS end=// TRAINING_DATA_MEMBERS_END

`_rval` and `_rvecval` can be used to access `Real` and `std::vector<Real>` response data, as specified by the `response` and `response_type` input parameters. `_pvals` can be used to access predictor data, gather from `Reporters` specified using the `predictors` input parameter. `_pcols` contains the `Sampler` column indices specified in the `predictor_cols` input. For `Trainers` that need to gather from other types (not `Real` or `std::vector<Real>`), `getTrainingData` should be called in the derived classes constructor. `SurrogateTrainer` also provides the global and local row indices, as well as the dimensionality of the predictor data.

## Declaring Training Data id=training-data

Model data must be declare in the object constructor using the `declareModelData` methods, which
are defined as follows. The desired type is provided as the template argument (`T`) and name to
the data is the first input parameter. The second option, if provided, is the initial value
for the training data. The name provided is arbitrary, but is used by the model object(s) designed
to work with the training data (see [Surrogates/index.md]).

!listing SurrogateTrainer.h start=MOOSEDOCS_BEGIN end=MOOSEDOCS_END include-start=False

These methods return a reference to the desired type that should be populated in the aforementioned
`train()` method. For example, in the [PolynomialChaosTrainer.md] trainer object a scalar value,
"order", is stored stored by declaring a reference to the desired type in the header.

!listing PolynomialChaosTrainer.h line=_order

Within the source the declared references are initialized with a declare method that includes
data initialization.

!listing PolynomialChaosTrainer.C line=_order(declare

The training data system leverages the [restart/Restartable.md] within MOOSE. As such, the data
store can be of an arbitrary type and is automatically used for restarting simulations.

## Enabling Cross Validation

K-fold cross validation options for `SurrogateTrainer` are in development. The current implementation requires a [Sampler](Samplers/index.md), `SurrogateTrainer`, and [SurrogateModel](Surrogates/index.md) to be defined in the input file. At the beginning of `execute()`, the trainer will shuffle and partition the rows of the provided [Sampler](Samplers/index.md) into $k$ folds. Then, in a loop over each fold, it will train the linked [SurrogateModel](Surrogates/index.md) and evaluate it for the test set. Training is performed using the same `preTrain()`, `train()`, and `postTrain()` methods as before. To make an existing trainer compatible with cross validation, `preTrain()` must reset the state of the trainer and clear any essential data related to prior training sets - for example, in [PolynomialRegressionTrainer.md] a linear system is assembled in the training loop and solved at the end in `postTrain()`. To enable cross validation, `preTrain()` was changed to reset the linear system.

!listing PolynomialRegressionTrainer.C re=void\sPolynomialRegressionTrainer.*?}

Because some indices are randomly skipped during training, insertion into pre-sized arrays by indexing operations during `train()` may inadvertently leave some zero values for the skipped rows. To avoid this, it may be more convenient to gather training data using `push_back`. If desired, memory can be reserved for these arrays using `getCurrentSampleSize()` or `getLocalSampleSize()`. An example of this approach is shown in [Creating a Surrogate Model](surrogate_creation.md).

During cross-validation the linked [SurrogateModel](Surrogates/index.md) will be used to generate predictions for the test set. To evaluate the error for these predictions, the `evaluateModelError` method is called. This method is declared virtual, so that it can be overridden to meet the needs of a particular implementation (for example, evaluations requiring pre-processing of predictor data). The default implementation covers the common anticipated uses cases (`Real` and `std::vector<Real>` responses). Developers of new trainer classes should consider whether their implementation should override this class or can use the default.

!listing SurrogateTrainer.C re=std::vector<Real>\sSurrogateTrainer::evaluateModelError.*?^}

For an example of the cross-validation system in use, see [/examples/cross_validation.md]

## Output Model Data

Training model data can be output to a binary file using the [SurrogateTrainerOutput.md] object.

## Example Input File Syntax

The following input file snippet adds a
[PolynomialChaosTrainer.md] object for training. Please refer to the documentation on the
individual models for more details.

!listing poly_chaos/main_2d_mc.i block=Trainers

!syntax list /Trainers objects=True actions=False subsystems=False

!syntax list /Trainers objects=False actions=False subsystems=True

!syntax list /Trainers objects=False actions=True subsystems=False
