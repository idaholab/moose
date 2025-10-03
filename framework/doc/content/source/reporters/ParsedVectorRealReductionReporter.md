# ParsedVectorRealReductionReporter

!syntax description /Reporters/ParsedVectorRealReductionReporter

## Overview

`ParsedVectorRealReductionReporter` performs a reduction on the elements of a vector in a reporter using a [parsed function expression](MooseParsedFunction.md) containing only two variables, `reduction_value` and `indexed_value`.  `reduction_value` is the variable containing the reduced value and is initialized by [!param](/Reporters/ParsedVectorRealReductionReporter/initial_reduction_value).  `indexed_value` is the current vector element being operated on by the parsed function.

Different initial conditions and [parsed function expressions](MooseParsedFunction.md) will provide a vector sum, vector multiplication, vector sum of squares, and max as shown in [vectorMath].

!listing parsed_reporters/vectorMath.i id=vectorMath block=Reporters/vec_d Reporters/vector_sum Reporters/vector_sqsum Reporters/vector_multiply Reporters/vector_max

## Optimization use case

`ParsedVectorRealReductionReporter` and [ParsedVectorVectorRealReductionReporter.md] were created to combine data from multiple subapps as shown in [sampler].
In this case, a `ParsedVectorRealReductionReporter` is needed to sum each subapp's parameter objective function returned by the `MultiAppReporterTransfer` into the `ConstantReporter` as a vector of vectors named `rec_vec`.
This set-up allows us to optimize parameters that combine multiple forward problems created and run in parallel using the `FullSolveMultiApp` system.
The scalar reporter computed by `ParsedVectorRealReductionReporter` is then be transferred as the objective value into [GeneralOptimization](source/optimizationreporters/GeneralOptimization.md optional=True).

!listing modules/optimization/test/tests/reporters/multiExperiment/sampler_subapp.i id=sampler block=Transfers Reporters

!syntax parameters /Reporters/ParsedVectorRealReductionReporter

!syntax inputs /Reporters/ParsedVectorRealReductionReporter

!syntax children /Reporters/ParsedVectorRealReductionReporter
