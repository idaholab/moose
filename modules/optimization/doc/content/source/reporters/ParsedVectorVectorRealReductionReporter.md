# ParsedVectorVectorRealReductionReporter

!syntax description /Reporters/ParsedVectorVectorRealReductionReporter

## Overview

`ParsedVectorVectorRealReductionReporter` performs a reduction across the rows of a vector of vectors contained in a reporter using a [parsed function expression](MooseParsedFunction.md) containing only two variables, `reduction_value` and `indexed_value`.  `reduction_value` is the variable containing the reduced value and is initialized by [!param](/Reporters/ParsedVectorVectorRealReductionReporter/initial_value).  `indexed_value` is the current vector element being operated on by the parsed function.  The output from `ParsedVectorVectorRealReductionReporter` is a column vector of the same size as the column size of the vector of vectors.  The vector of vectors reporter can be thought of as containing a matrix with the [parsed function expression](MooseParsedFunction.md) providing the row operation.  It is an error for the vectors contained in the vector of vectors to be different sizes.

Different initial conditions and [parsed function expressions](MooseParsedFunction.md) will provide different row operations on the matrix.  Examples are shown in [vectorMath] for a matrix row sum, matrix row multiplication, matrix row sum of squares, and max across the rows of a matrix.

!listing modules/optimization/test/tests/reporters/vector_math/vectorMath.i id=vectorMath block=Reporters/vec_d Reporters/vecvec_sum Reporters/vecvec_sqsum Reporters/vecvec_multiply Reporters/vecvec_max

## Optimization use case

`ParsedVectorVectorRealReductionReporter` and [ParsedVectorRealReductionReporter.md] were created to process data cloned into a `StochasticReporter` from a `SamplerReporterTransfer` as shown in [sampler].
In this case, a `ParsedVectorVectorRealReductionReporter` is needed to sum each sampler's parameter gradient vector returned to the `StochasticReporter` as a vector of vectors.
This set-up allows us to optimize parameters that combine multiple forward problems created and run in parallel using the sampler system.
The scalar reporter computed by `ParsedVectorVectorRealReductionReporter` is then be transferred as the objective value into [GeneralOptimization.md].

!listing test/tests/reporters/multiExperiment/sampler_subapp.i id=sampler block=Transfers Reporters

!syntax parameters /Reporters/ParsedVectorVectorRealReductionReporter

!syntax inputs /Reporters/ParsedVectorVectorRealReductionReporter

!syntax children /Reporters/ParsedVectorVectorRealReductionReporter
