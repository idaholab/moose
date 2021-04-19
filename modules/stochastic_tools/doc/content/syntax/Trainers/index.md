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

The method will execute once per execution flag (see [SetupInterface.md]) on each processor.
There are three virtual functions that derived class can and should override:

!listing SurrogateTrainer.h start=protected: end=postTrain include-start=False include-end=True

- `preTrain()` is called before the sampler loop and is typically used for resizing variables for the given number of data points.
- `train()` is called within the sampler loop where member variables `_local_row`, `_row`, and those declared with `getTrainingData` are updated.
- `postTrain()` is called after the sampler loop and is typically used for MPI communication.

## Gathering Training Data

In order to ease the of gathering the required data needed for training, `SurrogateTrainer`
includes API to get reporter data which takes care of the necessary size checks and
distributed data indexing.
The idea behind this is to emulate the element loop behavior in other MOOSE objects.
For instance, in a kernel, the value of _u corresponds to the solution in an element.
Here data referenced with `getTrainingData` will correspond to the the value of the
data in a sampler row. The returned reference is to be used in the `train()` function.
There are four functions that derived classes can call to gather training data:

!listing SurrogateTrainer.h start=TRAINING_DATA_BEGIN end=TRAINING_DATA_END include-start=False

- `getTrainingData<T>(const ReporterName & rname)` will get a vector of training data from a reporter value of type `std::vector<T>`, whose name is defined by `rname`.
- `getSamplerData()` will simply return a vector of the sampler row.


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

## Output Mdoel Data

Training model data can be output to a binary file using the [SurrogateTrainerOutput.md] object.

## Example Input File Syntax

The following input file snippet adds a
[PolynomialChaosTrainer.md] object for training. Please refer to the documentation on the
individual models for more details.

!listing poly_chaos/master_2d_mc.i block=Trainers

!syntax list /Trainers objects=True actions=False subsystems=False

!syntax list /Trainers objects=False actions=False subsystems=True

!syntax list /Trainers objects=False actions=True subsystems=False
