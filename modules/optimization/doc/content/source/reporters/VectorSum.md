# VectorSum

!syntax description /Reporters/VectorSum

## Overview

`VectorSum` operates on the elements of a vector in a reporter using a parsed function containing only two variables, `vi` and `vplus`.
The following algorithm applys the fu
vi=initial_value
for i=0:n
  vplus=v[i]
  vi=function(vi,vplus)

Using different intial conditions and parsed functions will provide a vector sum, vector multiplication, vector sum of squares, and max as shown in [vectorMath].

!listing modules/optimization/test/tests/reporters/vector_math/vectorMath.i id=vectorMath block=Reporters/vec_d Reporters/vector_sum Reporters/vector_sqsum Reporters/multiply Reporters/max


Like the [VectorOfVectorRowSum.md], this reporter was created to process data cloned into a `StochasticReporter` from a `SamplerReporterTransfer` as shown in [sampler].
In this case, A `VectorSum` is needed to sum each samplers objective function returned to the `StochasticReporter`.
This set-up allows us to optimize parameters that combine multiple forward problems created and run in parallel using the sampler system.
The scalar reporter created can then be transferred as the objective value into [GeneralOptimization.md].

!listing modules/combined/test/tests/optimization/invOpt_multiExperiment/sampler_subapp.i id=sampler block=Transfers Reporters

!syntax parameters /Reporters/VectorSum

!syntax inputs /Reporters/VectorSum

!syntax children /Reporters/VectorSum
