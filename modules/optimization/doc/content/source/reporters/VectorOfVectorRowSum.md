# VectorOfVectorRowSum

!syntax description /Reporters/VectorOfVectorRowSum

## Overview

`VectorOfVectorRowSum` consumes a reporter containing a vector of vectors of Real data and performs a parsed function across the element of those vectors.
The vectors being row summed must be the same size.
Like the [VectorSum.md], this reporter was created to process data cloned into a `StochasticReporter` from a `SamplerReporterTransfer` as shown in [sampler].
In this case, a `VectorOfVectorRowSum` is needed to sum each sampler's parameter gradient vector returned to the `StochasticReporter`.
This set-up allows us to optimize parameters that combine multiple forward problems created and run in parallel using the sampler system.
The vector reporter created can then be transferred as the parameter gradient in [GeneralOptimization.md].

!listing modules/combined/test/tests/optimization/invOpt_multiExperiment/sampler_subapp.i id=sampler block=Transfers Reporters

!syntax parameters /Reporters/VectorOfVectorRowSum

!syntax inputs /Reporters/VectorOfVectorRowSum

!syntax children /Reporters/VectorOfVectorRowSum
