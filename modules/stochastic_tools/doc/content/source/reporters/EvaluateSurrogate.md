# EvaluateSurrogate

!syntax description /Reporters/EvaluateSurrogate

## Overview

The EvaluateSurrogate object takes in a sampler and surrogate models and executes the `evaluate` method within each surrogate for each row of the sampler.
See [examples/surrogate_creation.md], [examples/surrogate_training.md], and [examples/surrogate_evaluate.md] for more information regarding surrogate modeling.

## Example Syntax

!listing modules/stochastic_tools/test/tests/surrogates/nearest_point/evaluate.i caption=Simple example using EvaluateSurrogate

!listing caption=CSV output
surrogate
8
8
24
40
56
72
88
104
120
136
...

!syntax parameters /Reporters/EvaluateSurrogate

!syntax inputs /Reporters/EvaluateSurrogate

!syntax children /Reporters/EvaluateSurrogate
