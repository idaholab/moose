# VectorOfVectorRowSum

!syntax description /Reporters/VectorOfVectorRowSum

## Overview

`VectorOfVectorRowSum` consumes a reporter containing a vector of vectors Real data and performs a row sum on those vectors.
The vectors being row summed must be the same size.
`VectorSum` sums the elements of a vector in a reporter and scales it by [!param](/Reporters/VectorSum/scale).
Like the [VectorSum.md], this reporter was created to process data cloned into a `StochasticReporter` from a `SamplerReporterTransfer` as shown in [sampler].
In this case, A `VectorOfVectorRowSum` is needed to sum each sampler's parameter gradient vector returned to the `StochasticReporter`.
This set-up allows us to optimize parameters that combine multiple forward problems created and run in parallel using the sampler system.
The vector reporter created can then be transferred as the parameter gradient in [GeneralOptimization.md].

!listing modules/combined/test/tests/optimization/invOpt_multiExperiment/sampler_subapp.i id=sampler block=Transfers Reporters

!syntax parameters /Reporters/VectorOfVectorRowSum

!syntax inputs /Reporters/VectorOfVectorRowSum

!syntax children /Reporters/VectorOfVectorRowSum
