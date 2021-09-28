# Surrogates System

## Overview

The Stochastic Tools module contains a system for training and running surrogate models. These
objects inherit from the `SurrogateModel` class and are added to a simulation using the
`[Surrogates]` block in the input file. `SurrogateModel` objects are standalone utilities designed to
be directly called by other objects in similar fashion to MOOSE
[Function objects](Functions/index.md), refer to [#using-model] for more information.

## Creating a SurrogateModel

A model is created by inheriting from `SurrogateModel` and overriding the evaluate method. This
same method is also what should be called by other objects requiring data. However, the model
classes can implement an arbitrary interface as needed.

## Using a SurrogateModel id=using-model

Model objects are obtained by other objects using the `getSurrogateModel` and
`getSurrogateModelByName` methods. The first expects an input parameter name from which the
desired object name is determined. The later function simply accepts the actual name of the
object.

For example, the [PolynomialChaosReporter.md] object requires a [PolynomialChaos.md] object. In the
header a reference to the desired model is declare and in the source this reference is initialized
with a get method.

!listing PolynomialChaosReporter.h line=std::vector<const PolynomialChaos *>

!listing PolynomialChaosReporter.C line=getSurrogateModel

## Getting Training Data id=training-data

Training data computed by a [trainer object](Trainers/index.md) is gathered using the `getModelData`
method. It has a template argument that gives the data type and a single argument, the name of the
training data. This name should match the name given in the associated training object(s). For
example, the "order" mentioned in the [PolynomialChaosTrainer.md] is needed in the actual model.
In the header a constant reference to the desired type is declared and this reference is initialized
in the source.

!listing PolynomialChaos.h line=_order

!listing PolynomialChaos.C line=_order(get

The training data can be supplied in one of two ways: using a file or directly from a trainer.

File based data initialization happens by using the [!param](/Surrogates/PolynomialChaos/filename).
The supplied file must have declared data with the same names and types as the get methods used
within this class. Refer to [SurrogateTrainerOutput.md] for more information.

It is also possible to use data directly from a trainer object by using the
[!param](/Surrogates/PolynomialChaos/trainer) parameter. This method directly shares the same
training data, so if the trainer updates the data the model has a reference to the same data.

## Example Input File Syntax

The following input file snippet adds a
[PolynomialChaos.md] surrogate model for evaluation. Please refer to the documentation on the
individual models for more details.

!listing poly_chaos/main_2d_mc.i block=Surrogates

!syntax list /Surrogates objects=True actions=False subsystems=False

!syntax list /Surrogates objects=False actions=False subsystems=True

!syntax list /Surrogates objects=False actions=True subsystems=False
