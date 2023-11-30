# VectorSum

!syntax description /Reporters/VectorSum

## Overview

`VectorSum` sums the elements of a vector in a reporter and scales it by [!param](/Reporters/VectorSum/scale).
Like the [VectorOfVectorRowSum.md], this reporter was created to process data cloned into a `StochasticReporter` from a `SamplerReporterTransfer` as shown in [sampler].
In this case, A `VectorSum` is needed to sum each samplers objective function returned to the `StochasticReporter`.
This set-up allows us to optimize parameters that combine multiple forward problems created and run in parallel using the sampler system.
The scalar reporter created can then be transferred as the objective value into [GeneralOptimization.md].

!listing modules/combined/test/tests/optimization/invOpt_multiExperiment/sampler_subapp.i id=sampler block=Transfers Reporters

!syntax parameters /Reporters/VectorSum

!syntax inputs /Reporters/VectorSum

!syntax children /Reporters/VectorSum
