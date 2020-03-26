# Trainers System

## Overview

Objects within the `[Trainers]` block are derived from `SurrogateTrainer` and
are designed for creating training data for use with a model (see [Surrogates/index.md]). The
trainer objects derive from the `GeneralUserObject`
class of MOOSE (see [UserObjects/index.md]).

## Creating a SurrogateTrainer

To create a trainer the new object should inherit from `SurrogateTrainer`, which are a part of the
MOOSE [UserObjects/index.md].  As a UserObject based tool a trainer object requires that the
`execute` method be overridden. The execute method should perform all the necessary calculations for
computing the training data, please refer to [#training-data] for details regarding declare training
data. The method will execute once per execution flag (see [SetupInterface.md]) on each processor.

## Declaring Training Data id=training-data

Training data must be declare in the object constructor using the `declareModelData` methods, which
are defined as follows. The desired type is provided as the template argument (`T`) and name to
the data is the first input parameter. The second option, if provided, is the initial value
for the training data. The name provided is arbitrary, but is used by the model object(s) designed
to work with the training data (see [Surrogates/index.md]).

!listing SurrogateTrainer.h start=MOOSEDOCS_BEGIN end=MOOSEDOCS_END include-start=False

These methods return a reference to the desired type that should be populated in the aforementioned
execute method. For example, in the [PolynomialChaosTrainer.md] trainer object a scalar value,
"order", is stored stored by declaring a reference to the desired type in the header.

!listing PolynomialChaosTrainer.h line=_order

Within the source the declared references are initialized with a declare method that includes
data initialization.

!listing PolynomialChaosTrainer.C line=_order(declare

The training data system leverages the [restart/Restartable.md] within MOOSE. As such, the data
store can be of an arbitrary type and is automatically used for restarting simulations.

## Output Training Data

Training data can be output to a binary file using the [SurrogateTrainerOutput.md] object.

## Example Input File Syntax

The following input file snippet adds a
[PolynomialChaosTrainer.md] object for training. Please refer to the documentation on the
individual models for more details.

!listing poly_chaos/master_2d_mc.i block=Trainers

!syntax list /Trainers objects=True actions=False subsystems=False

!syntax list /Trainers objects=False actions=False subsystems=True

!syntax list /Trainers objects=False actions=True subsystems=False
