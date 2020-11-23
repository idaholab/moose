# EvaluateGaussianProcess

!syntax description /VectorPostprocessors/EvaluateGaussianProcess

## Overview

Similar in function to [vectorpostprocessors/EvaluateSurrogate.md], this object takes in a sampler and a [GaussianProcess.md] surrogate model and executes the `evaluate` method within each surrogate for each row of the sampler. The `evaluate` return the predictive mean and standard deviation of the Gaussian process surrogate at the specified evaluation point.

## Example Syntax

!listing modules/stochastic_tools/test/tests/surrogates/gaussian_process/GP_squared_exponential_testing.i caption=Simple example using EvaluateGaussianProcess

!listing caption=CSV output when `output_samples = true`, note the predictive standard deviation in the last column.
surrogate,sample_p0,sample_p1,sample_p2
GP_avg,sample_p0,sample_p1,GP_avg_std
680.81518263199,8.1474396732963,9319.6492446266,0.58627430888001
1101.357289321,4.5170853933506,10872.970311667,0.58627427931307
787.59042890478,6.9085650921783,10118.299854788,0.58627437370922
722.81295049602,7.2617914444642,9222.6647540836,0.58627431013298
1127.8161258388,4.2903122598679,10668.108638239,0.58627425407458
1602.4960766395,2.7392312254796,10716.920553975,0.58627406227184
...

!syntax parameters /VectorPostprocessors/EvaluateGaussianProcess

!syntax inputs /VectorPostprocessors/EvaluateGaussianProcess

!syntax children /VectorPostprocessors/EvaluateGaussianProcess
