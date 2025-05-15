# GenericActiveLearningSampler

!syntax description /Samplers/GenericActiveLearningSampler

## Description

The `GenericActiveLearningSampler` is intended to facilitate parallel active learning schemes in MOOSE. This class does the following important functions:

- Propose `num_tries` Monte Carlo samples of the input parameters for evaluation by the [GaussianProcessSurrogate](GaussianProcessSurrogate.md).
- Take ranked indices from the previous iteration using `sorted_indices` to evaluate the subApp using the `num_parallel_proposals` best input parameters. The best set of input parameters is determined by the [GaussianProcessSurrogate](GaussianProcessSurrogate.md) and [ParallelAcquisition](ParallelAcquisition/index.md) systems.

This object should be used in conjuction to the Reporter [GenericActiveLearner](GenericActiveLearner.md) which facilitates the retraining of the Gaussian process and also picks the next best batch of inputs under which to evaluate the MOOSE model via the [AcquisitionFunction](Acquisitions/index.md). 

!syntax parameters /Samplers/GenericActiveLearningSampler

!syntax inputs /Samplers/GenericActiveLearningSampler

!syntax children /Samplers/GenericActiveLearningSampler
