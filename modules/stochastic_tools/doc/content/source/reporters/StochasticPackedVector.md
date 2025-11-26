# StochasticPackedVector

!syntax description /Reporters/StochasticPackedVector

## Overview

The StochasticPackedVector object allows users to pack multiple scalar sampler columns into a single vector-valued reporter, producing one `std::vector<Real>` per sample. This is especially useful for training multi-output surrogate models using user-generated data. For example, Gaussian Process models using the LMC covariance needs multiple response quantities combined into one vector per sample. The StochasticPackedVector reads rows from a specified Sampler object and collects selected columns into a single vector. Across all samples, this output is represented as `std::vector<std::vector<Real>>`.


## Example Syntax

The following is an example of using StochasticPackedVector to pack selected columns in sampler from user-generated data. Here we read the input and output parameters from a user-generated CSV file `sampler_data.csv`

!listing modules/stochastic_tools/test/tests/reporters/stochastic_packed_vector/stochastic_packed_vector.i block=Samplers

The input parameters are transferred via a [StochasticMatrix.md] and the output parameters are transferred and packed via current StochasticPackedVector object. This transfers the results in to the StochasticPackedVector object named `result_pair`.

!listing modules/stochastic_tools/test/tests/reporters/stochastic_packed_vector/stochastic_packed_vector.i block=Reporters

!syntax parameters /Reporters/StochasticPackedVector

!syntax inputs /Reporters/StochasticPackedVector

!syntax children /Reporters/StochasticPackedVector
