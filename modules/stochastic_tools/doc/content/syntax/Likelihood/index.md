# Likelihood System

## Overview

For performing Bayesian inference using MCMC techniques, a likelihood function needs to be defined for measuring the quality of model predictions with reference to the experiments. The likelihood functions can be defined in the `[Likelihood]` block.

## Creating a Likelihood Function

A likelihood function is created by inheriting from `LikelihoodFunctionBase` and `ReporterInterface` and overriding the `function` method in the base class. See the [Gaussian](Gaussian.md) class for an example.

## Example Input File Syntax

!listing test/tests/likelihoods/gaussian_derived/main.i block=Likelihood

!syntax list /Likelihood objects=True actions=False subsystems=False

!syntax list /Likelihood objects=False actions=False subsystems=True

!syntax list /Likelihood objects=False actions=True subsystems=False
