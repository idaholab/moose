# EvaluateSurrogate

!syntax description /VectorPostprocessors/EvaluateSurrogate

## Overview

The EvaluateSurrogate object takes in a sampler and surrogate models and executes the `evaluate` method within each surrogate for each row of the sampler.
See [examples/surrogate_creation.md], [examples/surrogate_training.md], and [examples/surrogate_evaluate.md] for more information regarding surrogate modeling.

For convenience, the parameter [!param](/VectorPostprocessors/EvaluateSurrogate/output_samples) is included so that the values from the sampler are included in the output.
Beware that setting this parameter to true can cause a large data output.

## Example Syntax

!listing modules/stochastic_tools/test/tests/surrogates/nearest_point/evaluate.i caption=Simple example using SurrogateEvaluate

!listing caption=CSV output when `output_samples = false`
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

!listing caption=CSV output when `output_samples = true`
surrogate,sample_p0,sample_p1,sample_p2
8,0.25,0.25,0.25
8,0.25,0.25,1.25
24,0.25,0.25,2.25
40,0.25,0.25,3.25
56,0.25,0.25,4.25
72,0.25,0.25,5.25
88,0.25,0.25,6.25
104,0.25,0.25,7.25
120,0.25,0.25,8.25
136,0.25,0.25,9.25
...

!syntax parameters /VectorPostprocessors/EvaluateSurrogate

!syntax inputs /VectorPostprocessors/EvaluateSurrogate

!syntax children /VectorPostprocessors/EvaluateSurrogate
