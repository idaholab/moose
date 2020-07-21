# Creating a Surrogate Model

This example goes through the process of creating a custom surrogate model, in his case the creation of [NearestPointSurrogate.md].

## Overview

Building a surrogate model requires the creation of two objects: SurrogateTrainer and SurrogateModel. The SurrogateTrainer uses information from samplers and results to construct variables to be saved into a `.rd` file at the conclusion of the training run. The SurrogateMode object loads the data from the `.rd` and contains a function called `evaluate` that evaluates the surrogate model at a given input. The SurrogateTrainer and Surrogate are heavily tied together where each have the same member variables, the difference being one saves the data and the other loads it. It might be beneficial to have an interface class that contains common functions for training and evaluating, to avoid duplicate code. This example will not go into the creation of this interface class.

## Creating a Trainer

This example will go over the creation of [NearestPointTrainer](NearestPointTrainer.md). [Trainers](Trainers/index.md) are technically a type of [GeneralUserObject.md] with the same `validParams`, `initialSetup`, `initialize`, `execute`, and `finalize` functions that are executed during the object's call.

### validParams

Typically, the trainer requires the input of a sampler, so that it understands what the inputs of the full-order model were run. The trainer also needs the output values from the full-order model which are stored in a [vector postprocessor](VectorPostprocessors/index.md).

!listing NearestPointTrainer.C re=InputParameters\sNearestPointTrainer::validParams.*?^}

### Constructor

All trainers are based on SurrogateTrainer, which provides the necessary interface for saving the surrogate model data. All the data meant to be saved is defined in the constructor of the training object. In [NearestPointTrainer](NearestPointTrainer.md), the variable `_sample_points` is declared as the necessary surrogate data, see [Trainers](Trainers/index.md) for more information on declaring model data:

!listing NearestPointTrainer.C re=NearestPointTrainer::NearestPointTrainer.*?^}

The member variable `_sample_points` is defined in the header file:

!listing NearestPointTrainer.h start=_sample_points end=_sample_points include-end=true

### initialSetup

Since [UserObjects](UserObjects/index.md) are constructed before [Samplers](Samplers/index.md) and [VectorPostprocessors](VectorPostprocessors/index.md), the sampler and vector postprocessor variables need to be set in `initialSetup`:

!listing NearestPointTrainer.C re=void\sNearestPointTrainer::initialSetup.*?^}

`_values_distributed` simply determines whether the values in the vector postprocessor are distributed, which necessitates different indices when looping through them. Since the definition of these variables is outside the constructor, they need to be pointers:

!listing NearestPointTrainer.h start=/// Sampler end=_values_ptr include-end=true

### initialize

`initialize` is called before `execute` is called for all [UserObjects](UserObjects/index.md). For [NearestPointTrainer.md], a check of the size of the sampler is performed and vector postprocessor and resize `_sample_points` appropriately:

!listing NearestPointTrainer.C re=void\sNearestPointTrainer::initialize.*?^}

Note that `getNumberOfRows()` is used to size the array, this is so that each processor contains a portion of the samples and results. We will gather all samples in `finalize`.

### execute

`execute` is where the actual training occurs. Here, a loop through the local processor's samples is performed to gather the parameter data and results:

!listing NearestPointTrainer.C re=void\sNearestPointTrainer::execute.*?^}

The `offset` variable is defined by whether or not the vector postprocessor values are distributed. Also, the `ind` variable defines the offset for saving the distributed samples.

### finalize

`finalize` is called after `execute` is called for all [UserObjects](UserObjects/index.md). This is typically where processor communication happens. Here, we use `finalize` to gather all the local `_sample_points` so that each processor has the full copy. `_communicator.allgather` makes it so that every processor has a copy of the full array and `_communicator.gather` makes it so that only one of the processors has the full copy, the latter is typically used because outputting only happens on the root processor. See [libMesh::Parallel::Communicator](http://libmesh.github.io/doxygen/classlibMesh_1_1Parallel_1_1Communicator.html) for more communication options.

!listing NearestPointTrainer.C re=void\sNearestPointTrainer::finalize.*?^}

## Creating a Surrogate

This example will go over the creation of [NearestPointSurrogate](NearestPointSurrogate.md). [Surrogates](Surrogates/index.md) are a specialized version of a [GeneralUserObject.md] that must have the `evaluate` public member function. The `validParams` for a surrogate will generally define how the surrogate is evaluated. [NearestPointSurrogate.md] does not have any options for the method of evaluation.

### Constructor

In the constructor, the references for the model data are defined, taken from the training data:

!listing NearestPointSurrogate.C re=NearestPointSurrogate::NearestPointSurrogate.*?^}

See [Surrogates](Surrogates/index.md) for more information on the `getModelData` function. `_sample_points` in the surrogate is a `const` reference, since we do not want to modify the training data during evaluation:

!listing NearestPointSurrogate.h start=/// Array end=sample_points include-end=true

### evaluate

`evaluate` is a public member function required for all surrogate models. This is where surrogate model is actually used. `evaluate` takes in parameter values and returns the surrogate's estimation of the quantity of interest. See [EvaluateSurrogate](EvaluateSurrogate.C) for an example on how the `evaluate` function is used.

!listing NearestPointSurrogate.C re=Real\sNearestPointSurrogate::evaluate.*?^}
