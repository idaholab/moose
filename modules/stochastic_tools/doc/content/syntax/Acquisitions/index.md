# ParallelAcquisition System

## Overview

For performing parallel active learning, an acquisition function is needed for prioritizing under what input conditions the high-fidelity model needs to be run. Once the high-fidelity model evaluations are performed, these outputs serve as additional training data for re-training the probabilistic ML model (e.g., Gaussian processes). The acquisition function can be defined in the `[ParallelAcquisition]` block.

## Creating an ParallelAcquisition function

An acquisition function is created by inheriting from `ParallelAcquisitionFunctionBase` and overriding the `computeAcquisition` method in the base class. See the [BayesianPosteriorTargeted](BayesianPosteriorTargeted.md) class for an example.

## Example Input File Syntax

!listing test/tests/likelihoods/gaussian_derived/main.i block=Likelihood

!syntax list /ParallelAcquisition objects=True actions=False subsystems=False

!syntax list /ParallelAcquisition objects=False actions=False subsystems=True

!syntax list /ParallelAcquisition objects=False actions=True subsystems=False