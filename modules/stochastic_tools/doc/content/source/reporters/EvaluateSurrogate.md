# EvaluateSurrogate

!syntax description /Reporters/EvaluateSurrogate

## Overview

The EvaluateSurrogate object takes in a sampler and surrogate models and executes the `evaluate` method within each surrogate for each row of the sampler.
See [examples/surrogate_creation.md], [examples/surrogate_training.md], and [examples/surrogate_evaluate.md] for more information regarding surrogate modeling.

## Example Syntax

!listing modules/stochastic_tools/test/tests/surrogates/nearest_point/evaluate.i caption=Simple example using EvaluateSurrogate

!listing surrogates/nearest_point/gold/evaluate_out_results_0002.csv caption=CSV output

!syntax parameters /Reporters/EvaluateSurrogate

!syntax inputs /Reporters/EvaluateSurrogate

!syntax children /Reporters/EvaluateSurrogate
