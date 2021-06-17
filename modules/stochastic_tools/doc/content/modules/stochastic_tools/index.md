# Stochastic Tools

The stochastic tools module is a toolbox designed for performing stochastic analysis for MOOSE-based
applications. The following sections detail the various aspects of this module that can be
used independently or in combination to meet the needs of the application developer.

## Examples

!include modules/stochastic_tools/stochastic_tools_examples.md

## Performance

The stochastic tools module is optimized in two ways for memory use. First, sub-applications can be
executed in batches and all objects utilizing sample data do so using a distributed sample
matrix. For further details refer to the following:

- [batch_mode.md]
- [distributed_samples.md]

## Objects, Actions, and Syntax

The following is a complete list of all objects available in the stochastic tools module.

!syntax complete groups=StochasticToolsApp
