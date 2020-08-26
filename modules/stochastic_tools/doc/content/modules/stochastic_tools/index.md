# Stochastic Tools

The stochastic tools module is a toolbox designed for performing stochastic analysis for MOOSE-based
applications. The following sections detail the various aspects of this module that can be
used independently or in combination to meet the needs of the application developer.

## Examples

### Parameter Studies, Statistics, and Sensitivity Analysis

- Example 1: [examples/monte_carlo.md]
- Example 2: [examples/parameter_study.md]
- Example 3: [examples/nonlin_parameter_study.md]
- Example 4: [examples/sobol.md]

### Surrogate Models

- Example 5:  [examples/surrogate_creation.md]
- Example 6: [examples/surrogate_training.md]
- Example 7: [examples/surrogate_evaluate.md]
- Example 8: [examples/poly_chaos_surrogate.md]
- Example 9: [examples/poly_regression_surrogate.md]
- Example 10: [examples/pod_rb_surrogate.md]
- Example 11: [examples/combined_example_2d_trans_diff.md]
- Example 12: [examples/gaussian_process_surrogate.md]

## Performance

The stochastic tools module is optimized in two ways for memory use. First, sub-applications can be
executed in batches and all objects utilizing sample data do so using a distributed sample
matrix. For further details refer to the following:

- [batch_mode.md]
- [distributed_samples.md]

## Objects, Actions, and Syntax

The following is a complete list of all objects available in the stochastic tools module.

!syntax complete groups=StochasticToolsApp
