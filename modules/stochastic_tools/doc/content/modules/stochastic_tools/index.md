# Stochastic Tools Module

The stochastic tools module is a toolbox designed for performing stochastic analysis for MOOSE-based
applications. The following sections detail the various aspects of this module that can be
used independently or in combination to meet the needs of the application developer.

## Examples

!include modules/stochastic_tools/examples/index.md start=example-lists-begin end=example-lists-end

## Performance

The stochastic tools module is optimized in two ways for memory use. First, sub-applications can be
executed in batches and all objects utilizing sample data do so using a distributed sample
matrix. For further details refer to the following:

- [batch_mode.md]
- [distributed_samples.md]

## Linking MOOSE with external Machine Learning libraries

The stochastic tools module provides neural network-based surrogate modeling capabilities
as well. However, to enable it one needs to compile MOOSE with the C++ APIs of
[pytorch](https://pytorch.org/). For this, follow the appropriate installation guide below:

- [enable_pytorch.md]

## Objects, Actions, and Syntax

The following is a complete list of all objects available in the stochastic tools module.

!syntax complete groups=StochasticToolsApp
