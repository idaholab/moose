# VectorOfVectorRowSum

!syntax description /Reporters/VectorOfVectorRowSum

## Overview

`VectorOfVectorRowSum` consumes a reporter containing a vector of vectors Real data and performs a row sum on those vectors.  The vectors being row summed must be the same size.  This reporter was created to sum vectors containing optimization parameter gradients that are cloned into a `StochasticReporter` from a `SamplerReporterTransfer`.  This set-up allows us to optimize parameters that combine multiple forward problems created and run in parallel using the sampler system.

!syntax parameters /Reporters/VectorOfVectorRowSum

!syntax inputs /Reporters/VectorOfVectorRowSum

!syntax children /Reporters/VectorOfVectorRowSum
