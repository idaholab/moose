# BayesianActiveLearningSampler

!syntax description /Samplers/BayesianActiveLearningSampler

## Description

The `BayesianActiveLearningSampler` derives off of [PMCMCBase](PMCMCBase.md) and is intended for parallel active learning for Bayesian inverse UQ problems. It's function is similar to that of [GenericActiveLearningSampler](GenericActiveLearningSampler.md). Differently, `BayesianActiveLearningSampler` considers the experimental configurations and combines them with the proposed samples in each iteration of active learning.

!syntax parameters /Samplers/BayesianActiveLearningSampler

!syntax inputs /Samplers/BayesianActiveLearningSampler

!syntax children /Samplers/BayesianActiveLearningSampler
