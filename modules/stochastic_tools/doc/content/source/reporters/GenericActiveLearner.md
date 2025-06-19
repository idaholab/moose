# GenericActiveLearner

!syntax description /Reporters/GenericActiveLearner

## Description

The `GenericActiveLearner` is intended to facilitate parallel active learning schemes in MOOSE. This class does the following important functions:

- Re-train the Gaussian process surrogate via the `reTrain` function in the [ActiveLearningGaussianProcess](ActiveLearningGaussianProcess.md) object.
- Select the next best batch of inputs on which to evaluate the MOOSE model using the [AcquisitionFunction](Acquisitions/index.md).

This object should be used in conjuction to the Reporter [GenericActiveLearningSampler](GenericActiveLearningSampler.md) which facilitates the generation of new samples for evaluating the Gaussian process and evaluation of the MOOSE model using the previous best samples selected by the Gaussian process.

!syntax parameters /Reporters/GenericActiveLearner

!syntax inputs /Reporters/GenericActiveLearner

!syntax children /Reporters/GenericActiveLearner
