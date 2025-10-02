# BayesianActiveLearner

!syntax description /Reporters/BayesianActiveLearner

## Description

The `BayesianActiveLearner` derives off of [GenericActiveLearner](GenericActiveLearner.md) and is intended for parallel active learning for Bayesian inverse UQ problems. It's function is similar to that of [GenericActiveLearner](GenericActiveLearner.md) with the following key differences:

- Via the [Likelihood](Likelihood/index.md), it considers the experimental data when evaluating the quality of Gaussian process predictions using [AcquisitionFunction](Acquisitions/index.md).
- When the user requests to also calibrate the variance term, additional mechanisms are built in to consider the variance term as an input to the Gaussian process in addition to the MOOSE model parameters.

!syntax parameters /Reporters/BayesianActiveLearner

!syntax inputs /Reporters/BayesianActiveLearner

!syntax children /Reporters/BayesianActiveLearner
